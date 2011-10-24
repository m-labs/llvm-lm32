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
#include "Mico32RegisterInfo.h"
#include "llvm/Module.h"
#include "llvm/Target/TargetMachine.h"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "Mico32GenSubtargetInfo.inc"

using namespace llvm;

Mico32Subtarget::Mico32Subtarget(const std::string &TT, 
                                 const std::string &CPU,
                                 const std::string &FS) 
  : Mico32GenSubtargetInfo(TT, CPU, FS)
{
  HasSDIV = false;
  HasDIV = true;
  HasMUL = true;
  HasBarrel = true;
  HasSPBias = true;

  // Parse features string.
  std::string CPUName = CPU;
  if (CPUName.empty())
    CPUName = "mico32";
  ParseSubtargetFeatures(CPUName, FS);
}
