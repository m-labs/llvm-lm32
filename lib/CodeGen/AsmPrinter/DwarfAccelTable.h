//==-- llvm/CodeGen/DwarfAccelTable.h - Dwarf Accelerator Tables -*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains support for writing dwarf accelerator tables.
//
//===----------------------------------------------------------------------===//

#ifndef CODEGEN_ASMPRINTER_DWARFACCELTABLE_H__
#define CODEGEN_ASMPRINTER_DWARFACCELTABLE_H__

#include "llvm/ADT/StringMap.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/FormattedStream.h"
#include <vector>
#include <map>

// The dwarf accelerator tables are an indirect hash table optimized
// for null lookup rather than access to known data. They are output into
// an on-disk format that looks like this:
//
// .-------------.
// |  HEADER     |
// |-------------|
// |  BUCKETS    |
// |-------------|
// |  HASHES     |
// |-------------|
// |  OFFSETS    |
// |-------------|
// |  DATA       |
// `-------------'
//
// where the header contains a magic number, version, type of hash function,
// the number of buckets, total number of hashes, and room for a special
// struct of data and the length of that struct.
//
// The buckets contain an index (e.g. 6) into the hashes array. The hashes
// section contains all of the 32-bit hash values in contiguous memory, and
// the offsets contain the offset into the data area for the particular
// hash.
// 
// For a lookup example, we could hash a function name and take it modulo the
// number of buckets giving us our bucket. From there we take the bucket value
// as an index into the hashes table and look at each successive hash as long
// as the hash value is still the same modulo result (bucket value) as earlier.
// If we have a match we look at that same entry in the offsets table and
// grab the offset in the data for our final match.

namespace llvm {

class AsmPrinter;
class DIE;
class DwarfDebug;
  
class DwarfAccelTable {

  enum HashFunctionType {
    eHashFunctionDJB = 0u
  };

  static uint32_t HashDJB (StringRef Str) {
    uint32_t h = 5381;
    for (unsigned i = 0, e = Str.size(); i != e; ++i)
      h = ((h << 5) + h) + Str[i];
    return h;
  }

  // Helper function to compute the number of buckets needed based on
  // the number of unique hashes.
  void ComputeBucketCount (void);
  
  struct TableHeader {
    uint32_t   magic;           // 'HASH' magic value to allow endian detection
    uint16_t   version;         // Version number.
    uint16_t   hash_function;   // The hash function enumeration that was used.
    uint32_t   bucket_count;    // The number of buckets in this hash table.
    uint32_t   hashes_count;    // The total number of unique hash values
                                // and hash data offsets in this table.
    uint32_t   header_data_len; // The bytes to skip to get to the hash
                                // indexes (buckets) for correct alignment.
    // Also written to disk is the implementation specific header data.

    static const uint32_t MagicHash = 0x48415348;
    
    TableHeader (uint32_t data_len) :
      magic (MagicHash), version (1), hash_function (eHashFunctionDJB),
      bucket_count (0), hashes_count (0), header_data_len (data_len)
    {}

#ifndef NDEBUG
    void print(raw_ostream &O) {
      O << "Magic: " << format("0x%x", magic) << "\n"
        << "Version: " << version << "\n"
        << "Hash Function: " << hash_function << "\n"
        << "Bucket Count: " << bucket_count << "\n"
        << "Header Data Length: " << header_data_len << "\n";
    }
    void dump() { print(dbgs()); }
#endif
  };

public:
  // The HeaderData describes the form of each set of data. In general this
  // is as a list of atoms (atom_count) where each atom contains a type
  // (AtomType type) of data, and an encoding form (form). In the case of
  // data that is referenced via DW_FORM_ref_* the die_offset_base is
  // used to describe the offset for all forms in the list of atoms.
  // This also serves as a public interface of sorts.
  // When written to disk this will have the form:
  //
  // uint32_t die_offset_base
  // uint32_t atom_count
  // atom_count Atoms  
  enum AtomType {
    eAtomTypeNULL       = 0u,
    eAtomTypeDIEOffset  = 1u,   // DIE offset, check form for encoding
    eAtomTypeCUOffset   = 2u,   // DIE offset of the compiler unit header that
                                // contains the item in question
    eAtomTypeTag        = 3u,   // DW_TAG_xxx value, should be encoded as
                                // DW_FORM_data1 (if no tags exceed 255) or
                                // DW_FORM_data2.
    eAtomTypeNameFlags  = 4u,   // Flags from enum NameFlags
    eAtomTypeTypeFlags  = 5u    // Flags from enum TypeFlags
  };

  // Make these public so that they can be used as a general interface to
  // the class.
  struct Atom {
    AtomType type; // enum AtomType
    uint16_t form; // DWARF DW_FORM_ defines

    Atom(AtomType type, uint16_t form) : type(type), form(form) {}
    static const char * AtomTypeString(enum AtomType);
#ifndef NDEBUG
    void print(raw_ostream &O) {
      O << "Type: " << dwarf::TagString(type) << "\n"
        << "Form: " << dwarf::FormEncodingString(form) << "\n";
    }
    void dump() {
      print(dbgs());
    }
#endif
  };

 private:
  struct TableHeaderData {
    
    uint32_t die_offset_base;
    std::vector<Atom> Atoms;

    TableHeaderData(DwarfAccelTable::Atom Atom, uint32_t offset = 0)
    : die_offset_base(offset) {
      Atoms.push_back(Atom);
    }
    
#ifndef NDEBUG
    void print (raw_ostream &O) {
      O << "die_offset_base: " << die_offset_base << "\n";
      for (size_t i = 0; i < Atoms.size(); i++)
        Atoms[i].print(O);
    }
    void dump() {
      print(dbgs());
    }
#endif
  };

  // The data itself consists of a str_offset, a count of the DIEs in the
  // hash and the offsets to the DIEs themselves.
  // On disk each data section is ended with a 0 KeyType as the end of the
  // hash chain.
  // On output this looks like:
  // uint32_t str_offset
  // uint32_t hash_data_count
  // HashData[hash_data_count]
  struct HashData {
    StringRef Str;
    uint32_t HashValue;
    MCSymbol *Sym;
    std::vector<uint32_t> DIEOffsets; // offsets
    HashData(StringRef S) : Str(S) {
      HashValue = DwarfAccelTable::HashDJB(S);
    }
    void addOffset(uint32_t off) { DIEOffsets.push_back(off); }
    #ifndef NDEBUG
    void print(raw_ostream &O) {
      O << "Name: " << Str << "\n";
      O << "  Hash Value: " << format("0x%x", HashValue) << "\n";
      O << "  Symbol: " ;
      if (Sym) Sym->print(O);
      else O << "<none>";
      O << "\n";
      for (size_t i = 0; i < DIEOffsets.size(); i++)
        O << "  Offset: " << DIEOffsets[i] << "\n";
    }
    void dump() {
      print(dbgs());
    }
    #endif
  };

  DwarfAccelTable(const DwarfAccelTable&); // DO NOT IMPLEMENT
  void operator=(const DwarfAccelTable&);  // DO NOT IMPLEMENT

  // Internal Functions
  void EmitHeader(AsmPrinter *);
  void EmitBuckets(AsmPrinter *);
  void EmitHashes(AsmPrinter *);
  void EmitOffsets(AsmPrinter *, MCSymbol *);
  void EmitData(AsmPrinter *, DwarfDebug *D);
  
  // Output Variables
  TableHeader Header;
  TableHeaderData HeaderData;
  std::vector<HashData*> Data;

  // String Data
  typedef std::vector<DIE*> DIEArray;
  typedef StringMap<DIEArray> StringEntries;
  StringEntries Entries;

  // Buckets/Hashes/Offsets
  typedef std::vector<HashData*> HashList;
  typedef std::vector<HashList> BucketList;
  BucketList Buckets;
  HashList Hashes;
  
  // Public Implementation
 public:
  DwarfAccelTable(DwarfAccelTable::Atom Atom);
  ~DwarfAccelTable();
  void AddName(StringRef, DIE*);
  void FinalizeTable(AsmPrinter *, const char *);
  void Emit(AsmPrinter *, MCSymbol *, DwarfDebug *);
#ifndef NDEBUG
  void print(raw_ostream &O);
  void dump() { print(dbgs()); }
#endif
};

}
#endif
