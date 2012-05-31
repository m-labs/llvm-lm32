//=== Mico32TargetMachine.h - Define TargetMachine for Mico32 -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Mico32 specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef MICO32TARGETMACHINE_H
#define MICO32TARGETMACHINE_H

#include "Mico32Subtarget.h"
#include "Mico32InstrInfo.h"
#include "Mico32ISelLowering.h"
#include "Mico32SelectionDAGInfo.h"
#include "Mico32FrameLowering.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetFrameLowering.h"


namespace llvm {
  class formatted_raw_ostream;
  
  class Mico32TargetMachine : public LLVMTargetMachine {
    Mico32Subtarget        Subtarget;
    const TargetData       DataLayout;       // Calculates type size & alignment
    Mico32InstrInfo        InstrInfo;
    Mico32FrameLowering    FrameLowering;
    Mico32TargetLowering   TLInfo;
    Mico32SelectionDAGInfo TSInfo;
    //Mico32ELFWriterInfo    ELFWriterInfo;
    
  public:
    Mico32TargetMachine(const Target &T, StringRef TT,
                        StringRef CPU, StringRef FS,
                        const TargetOptions &Options,
                        Reloc::Model RM, CodeModel::Model CM,
                        CodeGenOpt::Level OL);

    virtual const Mico32InstrInfo *getInstrInfo() const {
      return &InstrInfo;
    }
  
    virtual const TargetFrameLowering  *getFrameLowering() const {
      return &FrameLowering;
    }
  
    virtual const Mico32Subtarget *getSubtargetImpl() const {
      return &Subtarget;
    }
  
    virtual const Mico32RegisterInfo *getRegisterInfo() const {
      return &InstrInfo.getRegisterInfo();
    }
  
    virtual const Mico32TargetLowering* getTargetLowering() const {
      return &TLInfo;
    }
  
    virtual const Mico32SelectionDAGInfo* getSelectionDAGInfo() const {
      return &TSInfo;
    }
  
//    virtual const Mico32ELFWriterInfo *getELFWriterInfo() const {
//      return &ELFWriterInfo;
//    }
  
    virtual const TargetData *getTargetData() const {
      return &DataLayout;
    }
  
    // Pass Pipeline Configuration
    virtual bool addInstSelector(PassManagerBase &PM);
  };
  
} // end namespace llvm

#endif // MICO32TARGETMACHINE_H
