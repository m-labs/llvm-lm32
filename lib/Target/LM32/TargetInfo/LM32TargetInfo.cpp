//===-- LM32TargetInfo.cpp - LM32  Target Implementation ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "LM32.h"
#include "llvm/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target llvm::TheLM32Target;

extern "C" void LLVMInitializeLM32TargetInfo() {
  RegisterTarget<Triple::lm32> X(TheLM32Target, "lm32", "LM32");
}
