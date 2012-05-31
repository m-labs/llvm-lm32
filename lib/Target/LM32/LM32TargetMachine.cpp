//===-- LM32TargetMachine.cpp - Define TargetMachine for LM32 -------------===//
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

#include "LM32.h"
#include "LM32TargetMachine.h"
#include "llvm/PassManager.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;


extern "C" void LLVMInitializeLM32Target() {
  // Register the target.
  RegisterTargetMachine<LM32TargetMachine> X(TheLM32Target);
}

///
/// Note: DataLayout is described in:
/// http://www.llvm.org/docs/LangRef.html#datalayout
///
LM32TargetMachine::
LM32TargetMachine(const Target &T, StringRef TT,
                    StringRef CPU, StringRef FS,
                    const TargetOptions &Options,
                    Reloc::Model RM, CodeModel::Model CM,
                    CodeGenOpt::Level OL)
  : LLVMTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
    Subtarget(TT, CPU, FS),
//  DataLayout("E-p:32:32:32-i8:8:8-i16:16:16"),
//    DataLayout("E-p:32:32:32-i8:8:32-i16:16:32-i32:32:32-i64:32:32"),
//    DataLayout("E-p:32:32:32-i8:8:32-i16:16:32-i32:32:32-i64:32:32-a0:32:32-S32-s0:32:32-n32"),
// I seem to recall that floats and doubles aligned to less than their 
// natural alignment were getting realigned to the natural alignment by LLVM.
// I'm not sure why i64 was switched to i64 alignment from i32.
//    DataLayout("E-p:32:32:32-i8:8:32-i16:16:32-i32:32:32-i64:32:32-a0:8:8-S32-s0:32:32-n32"),
//    DataLayout("E-p:32:32:32-i8:8:32-i16:16:32-i32:32:32-i64:64:64-a0:8:8-S32-s0:32:32-n32"),
    // *** This must match Targets.cpp in clang.
    DataLayout("E-p:32:32:32-i8:8:32-i16:16:32-i32:32:32-i64:32:32-f32:32:32-f64:32:32-a0:8:8-S32-s0:32:32-n32"),
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
// the ISelDag to generate LM32 code.
bool LM32TargetMachine::addInstSelector(PassManagerBase &PM) {
  PM.add(createLM32ISelDag(*this));
  return false;
}
