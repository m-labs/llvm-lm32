//===-- Mico32SelectionDAGInfo.h - Mico32 SelectionDAG Info -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the Mico32 subclass for TargetSelectionDAGInfo.
//
//===----------------------------------------------------------------------===//

#ifndef MICO32SELECTIONDAGINFO_H
#define MICO32SELECTIONDAGINFO_H

#include "llvm/Target/TargetSelectionDAGInfo.h"

namespace llvm {

class Mico32TargetMachine;

class Mico32SelectionDAGInfo : public TargetSelectionDAGInfo {
public:
  explicit Mico32SelectionDAGInfo(const Mico32TargetMachine &TM);
  ~Mico32SelectionDAGInfo();
};

}

#endif
