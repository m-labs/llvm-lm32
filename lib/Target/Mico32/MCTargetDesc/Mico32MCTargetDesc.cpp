//==-- Mico32MCTargetDesc.cpp - Mico32 Target Descriptions --------*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides Mico32 specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "Mico32MCTargetDesc.h"
#include "Mico32MCAsmInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Target/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "Mico32GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "Mico32GenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "Mico32GenRegisterInfo.inc"

using namespace llvm;

static MCAsmInfo *createMCAsmInfo(const Target &T, StringRef TT) {
  Triple TheTriple(TT);
  switch (TheTriple.getOS()) {
  default:
    return new Mico32MCAsmInfo();
  }
}


static MCInstrInfo *createMico32MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitMico32MCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createMico32MCRegisterInfo(StringRef TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitMico32MCRegisterInfo(X, Mico32::RRA);
  return X;
}

static MCSubtargetInfo *createMico32MCSubtargetInfo(StringRef TT, StringRef CPU,
                                                   StringRef FS) {
  MCSubtargetInfo *X = new MCSubtargetInfo();
  InitMico32MCSubtargetInfo(X, TT, CPU, FS);
  return X;
}

static MCCodeGenInfo *createMico32MCCodeGenInfo(StringRef TT, Reloc::Model RM,
                                               CodeModel::Model CM) {
  MCCodeGenInfo *X = new MCCodeGenInfo();
  if (RM == Reloc::Default)  
    RM = Reloc::Static;
  if (CM == CodeModel::Default)
    CM = CodeModel::Small;
  X->InitMCCodeGenInfo(RM, CM);
  return X;
}

extern "C" void LLVMInitializeMico32TargetMC() {
  // Register the MC asm info.
  //RegisterMCAsmInfo<Mico32MCAsmInfo> X(TheMico32Target);
  RegisterMCAsmInfoFn X(TheMico32Target, createMCAsmInfo);

  // Register the MC codegen info.
  TargetRegistry::RegisterMCCodeGenInfo(TheMico32Target,
                                        createMico32MCCodeGenInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(TheMico32Target,
                                      createMico32MCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheMico32Target,
                                    createMico32MCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheMico32Target,
                                          createMico32MCSubtargetInfo);
}

