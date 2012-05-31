//===- LM32Subtarget.cpp - LM32 Subtarget Information -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the LM32 specific subclass of TargetSubtarget.
//
//===----------------------------------------------------------------------===//

#include "LM32Subtarget.h"
#include "LM32.h"
#include "LM32RegisterInfo.h"
#include "llvm/Module.h"
#include "llvm/Target/TargetMachine.h"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "LM32GenSubtargetInfo.inc"

using namespace llvm;

LM32Subtarget::LM32Subtarget(const std::string &TT, 
                                 const std::string &CPU,
                                 const std::string &FS) 
  : LM32GenSubtargetInfo(TT, CPU, FS)
{
  HasSDIV = false;
  HasDIV = true;
  HasMUL = true;
  HasBarrel = true;
  HasSPBias = true;

  // Parse features string.
  std::string CPUName = CPU;
  if (CPUName.empty())
    CPUName = "lm32";
  ParseSubtargetFeatures(CPUName, FS);
}
