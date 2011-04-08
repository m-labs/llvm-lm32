//===- Mico32Subtarget.cpp - Mico32 Subtarget Information -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Mico32 specific subclass of TargetSubtarget.
//
//===----------------------------------------------------------------------===//

#include "Mico32Subtarget.h"
#include "Mico32.h"
#include "llvm/Module.h"
#include "llvm/Target/TargetMachine.h"
#include "Mico32GenSubtarget.inc"
using namespace llvm;

Mico32Subtarget::Mico32Subtarget(const std::string &TT, const std::string &FS) 
{
  IsSDIVenabled = false;
  std::string CPU = "mico32";

  // Parse features string.
  ParseSubtargetFeatures(FS, CPU);

}
