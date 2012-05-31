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
#include "LM32ISelLowering.h"
#include "LM32SelectionDAGInfo.h"
#include "LM32FrameLowering.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetFrameLowering.h"


namespace llvm {
  class formatted_raw_ostream;
  
  class LM32TargetMachine : public LLVMTargetMachine {
    LM32Subtarget        Subtarget;
    const TargetData       DataLayout;       // Calculates type size & alignment
    LM32InstrInfo        InstrInfo;
    LM32FrameLowering    FrameLowering;
    LM32TargetLowering   TLInfo;
    LM32SelectionDAGInfo TSInfo;
    //LM32ELFWriterInfo    ELFWriterInfo;
    
  public:
    LM32TargetMachine(const Target &T, StringRef TT,
                        StringRef CPU, StringRef FS,
                        const TargetOptions &Options,
                        Reloc::Model RM, CodeModel::Model CM,
                        CodeGenOpt::Level OL);

    virtual const LM32InstrInfo *getInstrInfo() const {
      return &InstrInfo;
    }
  
    virtual const TargetFrameLowering  *getFrameLowering() const {
      return &FrameLowering;
    }
  
    virtual const LM32Subtarget *getSubtargetImpl() const {
      return &Subtarget;
    }
  
    virtual const LM32RegisterInfo *getRegisterInfo() const {
      return &InstrInfo.getRegisterInfo();
    }
  
    virtual const LM32TargetLowering* getTargetLowering() const {
      return &TLInfo;
    }
  
    virtual const LM32SelectionDAGInfo* getSelectionDAGInfo() const {
      return &TSInfo;
    }
  
//    virtual const LM32ELFWriterInfo *getELFWriterInfo() const {
//      return &ELFWriterInfo;
//    }
  
    virtual const TargetData *getTargetData() const {
      return &DataLayout;
    }
  
    // Pass Pipeline Configuration
    virtual bool addInstSelector(PassManagerBase &PM);
  };
  
} // end namespace llvm

#endif // LM32TARGETMACHINE_H
