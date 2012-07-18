//===-- LM32.h - Top-level interface for LM32 -------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in
// the LLVM LM32 back-end.
//
//===----------------------------------------------------------------------===//

#ifndef TARGET_LM32_H
#define TARGET_LM32_H

#include "MCTargetDesc/LM32MCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
  class LM32TargetMachine;
  class FunctionPass;
  class MachineCodeEmitter;
  class MCCodeEmitter;
  //class TargetAsmBackend;
  class formatted_raw_ostream;

  MCCodeEmitter *createLM32MCCodeEmitter(const Target &,
                                           TargetMachine &TM,
                                           MCContext &Ctx);

  //TargetAsmBackend *createLM32AsmBackend(const Target &, const std::string &);

  FunctionPass *createLM32ISelDag(LM32TargetMachine &TM);

  extern Target TheLM32Target;
} // end namespace llvm;

#endif
