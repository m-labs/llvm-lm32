//==-- Mico32MCTargetDesc.h - Mico32 Target Descriptions ---------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides Mico32 specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef MICO32MCTARGETDESC_H
#define MICO32MCTARGETDESC_H

namespace llvm {
class MCSubtargetInfo;
class Target;
class StringRef;

extern Target TheMico32Target;
extern Target TheMico32V9Target;

} // End llvm namespace

// Defines symbolic names for Mico32 registers.  This defines a mapping from
// register name to register number.
//
#define GET_REGINFO_ENUM
#include "Mico32GenRegisterInfo.inc"

// Defines symbolic names for the Mico32 instructions.
//
#define GET_INSTRINFO_ENUM
#include "Mico32GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "Mico32GenSubtargetInfo.inc"

#endif
