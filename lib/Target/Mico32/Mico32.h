//===-- Mico32.h - Top-level interface for Mico32 ---------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in
// the LLVM Mico32 back-end.
//
//===----------------------------------------------------------------------===//

#ifndef TARGET_MICO32_H
#define TARGET_MICO32_H

#include "llvm/Target/TargetMachine.h"

namespace llvm {
  class Mico32TargetMachine;
  class FunctionPass;
  class MachineCodeEmitter;
  class MCCodeEmitter;
  class TargetAsmBackend;
  class formatted_raw_ostream;

  MCCodeEmitter *createMico32MCCodeEmitter(const Target &,
                                           TargetMachine &TM,
                                           MCContext &Ctx);

  TargetAsmBackend *createMico32AsmBackend(const Target &, const std::string &);

  FunctionPass *createMico32ISelDag(Mico32TargetMachine &TM);

  extern Target TheMico32Target;
} // end namespace llvm;

// Defines symbolic names for Mico32 registers.  This defines a mapping from
// register name to register number.
#include "Mico32GenRegisterNames.inc"

// Defines symbolic names for the Mico32 instructions.
#include "Mico32GenInstrNames.inc"

#endif
