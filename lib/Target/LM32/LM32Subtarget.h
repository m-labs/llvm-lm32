//=====-- LM32Subtarget.h - Define Subtarget for the LM32 -----*- C++ -*--====//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the LM32 specific subclass of TargetSubtarget.
//
//===----------------------------------------------------------------------===//

#ifndef LM32SUBTARGET_H
#define LM32SUBTARGET_H

#include "LM32FrameLowering.h"
#include "LM32InstrInfo.h"
#include "LM32ISelLowering.h"
#include "LM32SelectionDAGInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include <string>

#define GET_SUBTARGETINFO_HEADER
#include "LM32GenSubtargetInfo.inc"


namespace llvm {
class StringRef;

class LM32Subtarget : public LM32GenSubtargetInfo {
  virtual void anchor();

  // Selected instruction itineraries (one entry per itinerary class.)
  // Used by LM32GenSubtarget.inc.
  InstrItineraryData InstrItins;
  const DataLayout DL;       // Calculates type size & alignment
  LM32InstrInfo InstrInfo;
  LM32FrameLowering FrameLowering;
  LM32TargetLowering TLInfo;
  LM32SelectionDAGInfo TSInfo;
  
  // Is the barrel shifter present?
  bool HasBarrel;
    
  // Are (undocumented) signed division 0x27 /modulo 0x35 instructions enabled.
  bool HasSDIV;
    
  // Are unsigned division/modulo instructions enabled.
  bool HasDIV;
  
  // Are multiply instructions enabled.
  bool HasMUL;
    
  // If true, SP points to first unused 32 bit word (the official ABI)
  // otherwise SP points to the last used word.
  bool HasSPBias;

public:

  /// This constructor initializes the data members to match that
  /// of the specified triple.
  LM32Subtarget(const std::string &TT, const std::string &CPU,
                const std::string &FS, const TargetMachine &TM);
    
  /// ParseSubtargetFeatures - Parses features string setting specified 
  /// subtarget options.  Definition of function is auto generated by tblgen.
  void ParseSubtargetFeatures(StringRef CPU, StringRef FS);

  bool hasBarrel()  const { return HasBarrel; };
  bool hasSDIV()    const { return HasSDIV; };
  bool hasDIV()     const { return HasDIV; };
  bool hasMUL()     const { return HasMUL; };
  bool hasSPBias()  const { return HasSPBias; };
    
  const LM32InstrInfo *getInstrInfo() const override { return &InstrInfo; }
  const TargetFrameLowering *getFrameLowering() const override {
      return &FrameLowering;
  }
  const LM32RegisterInfo *getRegisterInfo() const override {
      return &InstrInfo.getRegisterInfo();
  }
  const LM32TargetLowering *getTargetLowering() const override {
      return &TLInfo;
  }
  const LM32SelectionDAGInfo *getSelectionDAGInfo() const override {
      return &TSInfo;
  }
  const InstrItineraryData *getInstrItineraryData() const override {
      return &InstrItins;
  }
  const DataLayout *getDataLayout() const override { return &DL; }
    
};

} // End llvm namespace

#endif
