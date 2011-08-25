//===-- Mico32TargetInfo.cpp - Mico32  Target Implementation ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Mico32.h"
#include "llvm/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target llvm::TheMico32Target;

extern "C" void LLVMInitializeMico32TargetInfo() {
  RegisterTarget<Triple::mico32> X(TheMico32Target, "mico32", "Mico32");
}
