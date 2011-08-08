//===-- Mico32TargetMachine.cpp - Define TargetMachine for Mico32 ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "Mico32.h"
#include "Mico32TargetMachine.h"
#include "llvm/PassManager.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetRegistry.h"
using namespace llvm;


extern "C" void LLVMInitializeMico32Target() {
  // Register the target.
  RegisterTargetMachine<Mico32TargetMachine> X(TheMico32Target);
}

///
/// Note: DataLayout is described in:
/// http://www.llvm.org/docs/LangRef.html#datalayout
///
Mico32TargetMachine::
Mico32TargetMachine(const Target &T, StringRef TT,
                    StringRef CPU, StringRef FS,
                    Reloc::Model RM, CodeModel::Model CM)
  : LLVMTargetMachine(T, TT, CPU, FS, RM, CM),
    Subtarget(TT, CPU, FS),
    DataLayout("E-p:32:32:32-i8:8:8-i16:16:16"),
//    DataLayout("E-p:32:32:32-i8:8:32-i16:16:32"),
    InstrInfo(*this), 
    FrameLowering(Subtarget),
    TLInfo(*this), TSInfo(*this)
//    ELFWriterInfo(*this),
//    InstrItins(Subtarget.getInstrItineraryData())
{}

//===----------------------------------------------------------------------===//
// Pass Pipeline Configuration
//===----------------------------------------------------------------------===//


// Install an instruction selector pass using
// the ISelDag to generate Mico32 code.
bool Mico32TargetMachine::addInstSelector(PassManagerBase &PM,
                                          llvm::CodeGenOpt::Level OptLevel) {
  PM.add(createMico32ISelDag(*this));
  return false;
}
