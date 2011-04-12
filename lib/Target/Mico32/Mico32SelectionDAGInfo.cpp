//===-- Mico32SelectionDAGInfo.cpp - Mico32 SelectionDAG Info -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Mico32SelectionDAGInfo class.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "mico32-selectiondag-info"
#include "Mico32TargetMachine.h"
using namespace llvm;

Mico32SelectionDAGInfo::Mico32SelectionDAGInfo(const Mico32TargetMachine &TM)
  : TargetSelectionDAGInfo(TM) {
}

Mico32SelectionDAGInfo::~Mico32SelectionDAGInfo() {
}
