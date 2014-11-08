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
// I seem to recall that floats and doubles aligned to less than their 
// natural alignment were getting realigned to the natural alignment by LLVM.
// Note: must set both v64 and v128 since both are set in defaults.
// **** This must match Targets.cpp in clang. ****
    Layout("E-p:32:32:32-i8:8:32-i16:16:32-i32:32:32-i64:32:32-f32:32:32-f64:32:32-a0:8:32-S32-s0:32:32-n32-v64:32:32-v128:32:32"),
    InstrInfo(*this), 
    FrameLowering(Subtarget),
    TLInfo(*this), TSInfo(*this)
{}


//===----------------------------------------------------------------------===//
// Pass Pipeline Configuration
//===----------------------------------------------------------------------===//
namespace {
/// LM32 Code Generator Pass Configuration Options.
class LM32PassConfig : public TargetPassConfig {
public:
  LM32PassConfig(LM32TargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  LM32TargetMachine &getLM32TargetMachine() const {
    return getTM<LM32TargetMachine>();
  }

  virtual bool addInstSelector();
};
} // namespace

TargetPassConfig *LM32TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new LM32PassConfig(this, PM);
}

bool LM32PassConfig::addInstSelector() {
  addPass(createLM32ISelDag(getLM32TargetMachine(), getOptLevel()));
  return false;
}

// Force static initialization.
extern "C" void LLVMInitializeLM32Target() {
  RegisterTargetMachine<LM32TargetMachine> X(TheLM32Target);
}
