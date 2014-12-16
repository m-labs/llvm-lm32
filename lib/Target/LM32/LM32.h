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

#ifndef LLVM_LIB_TARGET_LM32_LM32_H
#define LLVM_LIB_TARGET_LM32_LM32_H

#include "MCTargetDesc/LM32MCTargetDesc.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Target/TargetMachine.h"


namespace llvm {
  class FunctionPass;
  class ModulePass;
  class TargetMachine;
  class MachineInstr;
  class LM32MCInst;
  class LM32AsmPrinter;
  class LM32TargetMachine;
  class formatted_raw_ostream;
  class raw_ostream;

/* TODO: object output.
  MCCodeEmitter *createLM32MCCodeEmitter(const Target &,
                                           TargetMachine &TM,
                                           MCContext &Ctx);
*/
/* TODO: assembler input.
 TargetAsmBackend *createHexagonAsmBackend(const Target &,
 const std::string &);
*/
  FunctionPass *createLM32ISelDag(LM32TargetMachine &TM,
                                  CodeGenOpt::Level OptLevel);

} // end namespace llvm;

#endif  // LLVM_LIB_TARGET_LM32_LM32_H
