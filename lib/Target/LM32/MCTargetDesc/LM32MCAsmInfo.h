//====-- LM32MCAsmInfo.h - LM32 asm properties -----------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the LM32MCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LM32_MCTARGETDESC_LM32MCASMINFO_H
#define LLVM_LIB_TARGET_LM32_MCTARGETDESC_LM32MCASMINFO_H

#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {

  class LM32MCAsmInfo : public MCAsmInfoELF {
    void anchor() override;
  public:
    explicit LM32MCAsmInfo(StringRef TT);
  };

} // namespace llvm

#endif  // LLVM_LIB_TARGET_LM32_MCTARGETDESC_LM32MCASMINFO_H
