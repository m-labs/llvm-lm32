//==-- LM32MCTargetDesc.h - LM32 Target Descriptions -------------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides LM32 specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LM32MCTARGETDESC_H
#define LM32MCTARGETDESC_H

namespace llvm {
class MCSubtargetInfo;
class Target;
class StringRef;

extern Target TheLM32Target;
extern Target TheLM32V9Target;

} // End llvm namespace

// Defines symbolic names for LM32 registers.  This defines a mapping from
// register name to register number.
//
#define GET_REGINFO_ENUM
#include "LM32GenRegisterInfo.inc"

// Defines symbolic names for the LM32 instructions.
//
#define GET_INSTRINFO_ENUM
#include "LM32GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "LM32GenSubtargetInfo.inc"

#endif
