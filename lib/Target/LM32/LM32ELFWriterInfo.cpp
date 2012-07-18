//===-- LM32ELFWriterInfo.cpp - ELF Writer Info for the LM32 backend --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements ELF writer information for the LM32 backend.
//
//===----------------------------------------------------------------------===//

#include "LM32ELFWriterInfo.h"
#include "LM32Relocations.h"
#include "llvm/Function.h"
#include "llvm/Support/ELF.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

//===----------------------------------------------------------------------===//
//  Implementation of the LM32ELFWriterInfo class
//===----------------------------------------------------------------------===//

LM32ELFWriterInfo::LM32ELFWriterInfo(TargetMachine &TM)
  : TargetELFWriterInfo(TM.getTargetData()->getPointerSizeInBits() == 64,
                        TM.getTargetData()->isLittleEndian()) {
}

LM32ELFWriterInfo::~LM32ELFWriterInfo() {}

unsigned LM32ELFWriterInfo::getRelocationType(unsigned MachineRelTy) const {
llvm_unreachable("unknown lm32 relocation type");
  switch (MachineRelTy) {
  case LM32::reloc_pcrel_word:
    return ELF::R_MICROBLAZE_64_PCREL;
  case LM32::reloc_absolute_word:
    return ELF::R_MICROBLAZE_NONE;
  default:
    llvm_unreachable("unknown lm32 machine relocation type");
  }
  return 0;
}

long int LM32ELFWriterInfo::getDefaultAddendForRelTy(unsigned RelTy,
                                                    long int Modifier) const {
llvm_unreachable("unknown lm32 relocation type");
  switch (RelTy) {
  case ELF::R_MICROBLAZE_32_PCREL:
    return Modifier - 4;
  case ELF::R_MICROBLAZE_32:
    return Modifier;
  default:
    llvm_unreachable("unknown lm32 relocation type");
  }
  return 0;
}

unsigned LM32ELFWriterInfo::getRelocationTySize(unsigned RelTy) const {
llvm_unreachable("unknown lm32 relocation type");
  // FIXME: Most of these sizes are guesses based on the name
  switch (RelTy) {
  case ELF::R_MICROBLAZE_32:
  case ELF::R_MICROBLAZE_32_PCREL:
  case ELF::R_MICROBLAZE_32_PCREL_LO:
  case ELF::R_MICROBLAZE_32_LO:
  case ELF::R_MICROBLAZE_SRO32:
  case ELF::R_MICROBLAZE_SRW32:
  case ELF::R_MICROBLAZE_32_SYM_OP_SYM:
  case ELF::R_MICROBLAZE_GOTOFF_32:
    return 32;

  case ELF::R_MICROBLAZE_64_PCREL:
  case ELF::R_MICROBLAZE_64:
  case ELF::R_MICROBLAZE_GOTPC_64:
  case ELF::R_MICROBLAZE_GOT_64:
  case ELF::R_MICROBLAZE_PLT_64:
  case ELF::R_MICROBLAZE_GOTOFF_64:
    return 64;
  }

  return 0;
}

bool LM32ELFWriterInfo::isPCRelativeRel(unsigned RelTy) const {
llvm_unreachable("unknown lm32 relocation type");
  // FIXME: Most of these are guesses based on the name
  switch (RelTy) {
  case ELF::R_MICROBLAZE_32_PCREL:
  case ELF::R_MICROBLAZE_64_PCREL:
  case ELF::R_MICROBLAZE_32_PCREL_LO:
  case ELF::R_MICROBLAZE_GOTPC_64:
    return true;
  }

  return false;
}

unsigned LM32ELFWriterInfo::getAbsoluteLabelMachineRelTy() const {
llvm_unreachable("unknown lm32 relocation type");
  return LM32::reloc_absolute_word;
}

long int LM32ELFWriterInfo::computeRelocation(unsigned SymOffset,
                                                unsigned RelOffset,
                                                unsigned RelTy) const {
llvm_unreachable("unknown lm32 relocation type");
  if (RelTy == ELF::R_MICROBLAZE_32_PCREL || ELF::R_MICROBLAZE_64_PCREL)
    return SymOffset - (RelOffset + 4);
  else
    assert("computeRelocation unknown for this relocation type");

  return 0;
}
