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
#include "LM32MCAsmInfo.h"
#include "llvm/MC/MCCodeGenInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "LM32GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "LM32GenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "LM32GenRegisterInfo.inc"

using namespace llvm;

static MCInstrInfo *createLM32MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitLM32MCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createLM32MCRegisterInfo(StringRef TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitLM32MCRegisterInfo(X, LM32::RRA);
  return X;
}

static MCSubtargetInfo *createLM32MCSubtargetInfo(StringRef TT, StringRef CPU,
                                                   StringRef FS) {
  MCSubtargetInfo *X = new MCSubtargetInfo();
  InitLM32MCSubtargetInfo(X, TT, CPU, FS);
  return X;
}


static MCAsmInfo *createLM32MCAsmInfo(const Target &T, StringRef TT) {
  MCAsmInfo *MAI = new LM32MCAsmInfo(T, TT);

  // Set the initial debugging machine moves that are assumed
  // on entry to a function.  From XCore.
  // Initial frame pointer is SP.
  MachineLocation Dst(MachineLocation::VirtualFP);
  MachineLocation Src(LM32::RSP, 0);
  MAI->addInitialFrameState(0, Dst, Src);

  return MAI;
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

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(TheLM32Target,
                                      createLM32MCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheLM32Target,
                                    createLM32MCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheLM32Target,
                                          createLM32MCSubtargetInfo);
}

