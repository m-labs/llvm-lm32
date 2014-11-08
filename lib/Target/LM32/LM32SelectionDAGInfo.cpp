//===-- LM32SelectionDAGInfo.cpp - LM32 SelectionDAG Info -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the LM32SelectionDAGInfo class.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "lm32-selectiondag-info"
#include "LM32TargetMachine.h"
using namespace llvm;

LM32SelectionDAGInfo::LM32SelectionDAGInfo(const LM32TargetMachine &TM)
  : TargetSelectionDAGInfo(TM) {
}

LM32SelectionDAGInfo::~LM32SelectionDAGInfo() {
}
