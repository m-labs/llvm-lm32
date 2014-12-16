//=== LM32TargetMachine.h - Define TargetMachine for LM32 -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the LM32 specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LM32TARGETMACHINE_H
#define LM32TARGETMACHINE_H

#include "LM32Subtarget.h"
#include "LM32InstrInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetFrameLowering.h"


namespace llvm {
  
class LM32TargetMachine : public LLVMTargetMachine {
    std::unique_ptr<TargetLoweringObjectFile> TLOF;
    LM32Subtarget Subtarget;
public:
    LM32TargetMachine(const Target &T, StringRef TT,
                        StringRef CPU, StringRef FS,
                        const TargetOptions &Options,
                        Reloc::Model RM, CodeModel::Model CM,
                        CodeGenOpt::Level OL);
    ~LM32TargetMachine() override;
    
    const LM32Subtarget *getSubtargetImpl() const override { return &Subtarget; }

    // Pass Pipeline Configuration
    TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
    TargetLoweringObjectFile *getObjFileLowering() const override {
        return TLOF.get();
    }
    
};
  
} // end namespace llvm

#endif // LM32TARGETMACHINE_H
