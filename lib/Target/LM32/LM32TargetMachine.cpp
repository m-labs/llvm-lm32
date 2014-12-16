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

#include "LM32TargetMachine.h"
#include "LM32.h"
#include "LM32ISelLowering.h"
#include "LM32TargetObjectFile.h"

#include "llvm/PassManager.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;


LM32TargetMachine::
LM32TargetMachine(const Target &T, StringRef TT,
                    StringRef CPU, StringRef FS,
                    const TargetOptions &Options,
                    Reloc::Model RM, CodeModel::Model CM,
                    CodeGenOpt::Level OL)
  : LLVMTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
    TLOF(make_unique<LM32TargetObjectFile>()),
    Subtarget(TT, CPU, FS, *this)
{
    initAsmInfo();
}

LM32TargetMachine::~LM32TargetMachine() {}


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
