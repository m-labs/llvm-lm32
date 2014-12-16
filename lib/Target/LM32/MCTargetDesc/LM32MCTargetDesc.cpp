//==-- LM32MCTargetDesc.cpp - LM32 Target Descriptions ------------*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides LM32 specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "LM32MCTargetDesc.h"
#include "InstPrinter/LM32InstPrinter.h"
#include "LM32MCAsmInfo.h"
#include "llvm/MC/MCCodeGenInfo.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MachineLocation.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "LM32GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "LM32GenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "LM32GenRegisterInfo.inc"


static MCInstPrinter *createLM32MCInstPrinter(const Target &T,
                                                 unsigned SyntaxVariant,
                                                 const MCAsmInfo &MAI,
                                                 const MCInstrInfo &MII,
                                                 const MCRegisterInfo &MRI,
                                                 const MCSubtargetInfo &STI) {
    return new LM32InstPrinter(MAI, MII, MRI);
}

static MCInstrInfo *createLM32MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitLM32MCInstrInfo(X);
  return X;
}

static MCRegisterInfo *
createLM32MCRegisterInfo(StringRef TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitLM32MCRegisterInfo(X, LM32::RRA);
  return X;
}

static MCSubtargetInfo *
createLM32MCSubtargetInfo(StringRef TT, StringRef CPU, StringRef FS) {
  MCSubtargetInfo *X = new MCSubtargetInfo();
  InitLM32MCSubtargetInfo(X, TT, CPU, FS);
  return X;
}

static MCAsmInfo *createLM32MCAsmInfo(const MCRegisterInfo &MRI, StringRef TT) {
  MCAsmInfo *MAI = new LM32MCAsmInfo(TT);

  // Set the initial debugging machine moves that are assumed
  // on entry to a function.  From XCore.n
  // Initial state of the frame pointer is SP.
  MCCFIInstruction Inst = MCCFIInstruction::createDefCfa(nullptr, LM32::RSP, 0);
  MAI->addInitialFrameState(Inst);

  return MAI;
}

static MCStreamer *
createLM32ELFStreamer(MCContext &Context, MCAsmBackend &MAB,
                      raw_ostream &OS, MCCodeEmitter *CE,
                      bool RelaxAll) {
    MCELFStreamer *ES = new MCELFStreamer(Context, MAB, OS, CE);
    return ES;
}


static MCStreamer *createLM32MCObjStreamer(Target const &T, StringRef TT,
                                    MCContext &Context, MCAsmBackend &MAB,
                                    raw_ostream &OS, MCCodeEmitter *Emitter,
                                    MCSubtargetInfo const &STI, bool RelaxAll) {
    MCStreamer *ES = createLM32ELFStreamer(Context, MAB, OS, Emitter, RelaxAll);
    new MCTargetStreamer(*ES);
    return ES;
}

static MCCodeGenInfo *createLM32MCCodeGenInfo(StringRef TT, Reloc::Model RM,
                                                CodeModel::Model CM,
                                                CodeGenOpt::Level OL) {
  MCCodeGenInfo *X = new MCCodeGenInfo();
  if (RM == Reloc::Default)  
    RM = Reloc::Static;
  if (CM == CodeModel::Default)
    CM = CodeModel::Small;
  X->InitMCCodeGenInfo(RM, CM, OL);
  return X;
}

extern "C" void LLVMInitializeLM32TargetMC() {
  // Register the MC asm info.
  //RegisterMCAsmInfo<LM32MCAsmInfo> X(TheLM32Target);
  RegisterMCAsmInfoFn X(TheLM32Target, createLM32MCAsmInfo);

  // Register the MC codegen info.
  TargetRegistry::RegisterMCCodeGenInfo(TheLM32Target,
                                        createLM32MCCodeGenInfo);
    
  // Register the object streamer.
  TargetRegistry::RegisterMCObjectStreamer(TheLM32Target,
                                           createLM32MCObjStreamer);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(TheLM32Target,
                                      createLM32MCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheLM32Target,
                                    createLM32MCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheLM32Target,
                                          createLM32MCSubtargetInfo);
    
  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(TheLM32Target,
                                        createLM32MCInstPrinter);
    

}

