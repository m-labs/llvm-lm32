//===-- Mico32MCAsmInfo.cpp - Mico32 asm properties -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the Mico32MCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "Mico32MCAsmInfo.h"
using namespace llvm;

Mico32MCAsmInfo::Mico32MCAsmInfo(const Target &T, StringRef TT)
{
  IsLittleEndian = false;
// These are from MBlaze.
  SupportsDebugInformation    = true;
//  AlignmentIsInBytes          = false;
  AlignmentIsInBytes          = true;
  PrivateGlobalPrefix         = ".L";
  GPRel32Directive            = "\t.gpword\t";

  // Uses '.section' before '.bss' directive
  UsesELFSectionDirectiveForBSS = true;


#if 0
// These are from Monarch.
  // Monarch supports emission of Dwarf debugging information.
  SupportsDebugInformation = true;
  HasLEB128 = true; // Target asm supports leb128 directives (little-endian)


  // These are from PowerPC (there were others not included too):
  CommentString = "#";
  GlobalPrefix = "";
  PrivateGlobalPrefix = ".L";
  WeakRefDirective = "\t.weak\t";

  // Uses '.section' before '.bss' directive
  UsesELFSectionDirectiveForBSS = true;

  // Align directives are power of 2.
  AlignmentIsInBytes = false;
  PCSymbol=".";


  InlineAsmStart = "## InlineAsm Start";
  InlineAsmEnd = "## InlineAsm End";


This is from the old MonarchTargetAsmInfo.cpp
  // We'll use all the default values for now defined 
  // in TargetAsmInfo and ELFTargetAsmInfo and see what fails.
  InlineAsmStart = "## InlineAsm Start";
  InlineAsmEnd = "## InlineAsm End";
  // This is required (no default).
  CStringSection = ".rodata.str";
  //CStringSection = "\t.cstring";
  WeakRefDirective = "\t.weak\t";
#endif
}



