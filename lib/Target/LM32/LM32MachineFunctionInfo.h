//=== LM32MachineFuctionInfo.h - LM32 machine function info -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares LM32-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef LM32MACHINEFUNCTIONINFO_H
#define LM32MACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include <vector>

namespace llvm {

// Forward declarations
class Function;

/// LM32FunctionInfo - This class is derived from MachineFunction private
/// LM32 target-specific information for each MachineFunction.
class LM32FunctionInfo : public MachineFunctionInfo {
private:
  bool UsesLR;
  int LRSpillSlot;
  int FPSpillSlot;
  int VarArgsFrameIndex;
  std::vector<std::pair<MCSymbol*, CalleeSavedInfo> > SpillLabels;

public:
  LM32FunctionInfo() :
    UsesLR(false),
    LRSpillSlot(0),
    FPSpillSlot(0),
    VarArgsFrameIndex(0) {}
  
  explicit LM32FunctionInfo(MachineFunction &MF) :
    UsesLR(false),
    LRSpillSlot(0),
    FPSpillSlot(0),
    VarArgsFrameIndex(0) {}
  
  ~LM32FunctionInfo() {}
  
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

#endif // LM32MACHINEFUNCTIONINFO_H
