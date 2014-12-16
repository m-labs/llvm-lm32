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
#include "llvm/Support/TargetRegistry.h"


#define DEBUG_TYPE "lm32-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "LM32GenSubtargetInfo.inc"

using namespace llvm;

void LM32Subtarget::anchor() { }

LM32Subtarget::LM32Subtarget(const std::string &TT, const std::string &CPU,
                             const std::string &FS, const TargetMachine &TM)
  : LM32GenSubtargetInfo(TT, CPU, FS),
    // Note: DataLayout is described in:
    // http://www.llvm.org/docs/LangRef.html#datalayout
    //
    // I seem to recall that floats and doubles aligned to less than their
    // natural alignment were getting realigned to the natural alignment by LLVM.
    // Note: must set both v64 and v128 since both are set in defaults.
    // **** This must match Targets.cpp in clang. ****
    DL("E-m:e-p:32:32:32-i8:8:32-i16:16:32-i32:32:32-i64:32:32-f32:32:32-f64:32:32-a0:8:32-S32-s0:32:32-n32-v64:32:32-v128:32:32"),
    InstrInfo(*this),
    FrameLowering(*this),
    TLInfo(TM),
    TSInfo(DL),
    HasBarrel(false),
    HasSDIV(false),
    HasDIV(false),
    HasMUL(false),
    HasSPBias(false)
{

  // Parse features string.
  std::string CPUName = CPU;
  if (CPUName.empty())
    CPUName = "lm32";
  ParseSubtargetFeatures(CPUName, FS);
}
