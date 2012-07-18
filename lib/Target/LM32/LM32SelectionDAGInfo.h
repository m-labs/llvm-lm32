//===-- LM32SelectionDAGInfo.h - LM32 SelectionDAG Info ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the LM32 subclass for TargetSelectionDAGInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LM32SELECTIONDAGINFO_H
#define LM32SELECTIONDAGINFO_H

#include "llvm/Target/TargetSelectionDAGInfo.h"

namespace llvm {

class LM32TargetMachine;

class LM32SelectionDAGInfo : public TargetSelectionDAGInfo {
public:
  explicit LM32SelectionDAGInfo(const LM32TargetMachine &TM);
  ~LM32SelectionDAGInfo();
};

}

#endif
