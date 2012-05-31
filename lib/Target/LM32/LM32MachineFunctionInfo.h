//=== Mico32MachineFuctionInfo.h - Mico32 machine function info -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares Mico32-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef MICO32MACHINEFUNCTIONINFO_H
#define MICO32MACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include <vector>

namespace llvm {

// Forward declarations
class Function;

/// Mico32FunctionInfo - This class is derived from MachineFunction private
/// Mico32 target-specific information for each MachineFunction.
class Mico32FunctionInfo : public MachineFunctionInfo {
private:
  bool UsesLR;
  int LRSpillSlot;
  int FPSpillSlot;
  int VarArgsFrameIndex;
  std::vector<std::pair<MCSymbol*, CalleeSavedInfo> > SpillLabels;

public:
  Mico32FunctionInfo() :
    UsesLR(false),
    LRSpillSlot(0),
    FPSpillSlot(0),
    VarArgsFrameIndex(0) {}
  
  explicit Mico32FunctionInfo(MachineFunction &MF) :
    UsesLR(false),
    LRSpillSlot(0),
    FPSpillSlot(0),
    VarArgsFrameIndex(0) {}
  
  ~Mico32FunctionInfo() {}
  
  void setVarArgsFrameIndex(int idx) { VarArgsFrameIndex = idx; }
  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  
  void setUsesLR(bool val) { UsesLR = val; }
  bool getUsesLR() const { return UsesLR; }
  
  void setLRSpillSlot(int off) { LRSpillSlot = off; }
  int getLRSpillSlot() const { return LRSpillSlot; }
  
  void setFPSpillSlot(int off) { FPSpillSlot = off; }
  int getFPSpillSlot() const { return FPSpillSlot; }
  
  std::vector<std::pair<MCSymbol*, CalleeSavedInfo> > &getSpillLabels() {
    return SpillLabels;
  }
};
} // End llvm namespace

#endif // MICO32MACHINEFUNCTIONINFO_H
