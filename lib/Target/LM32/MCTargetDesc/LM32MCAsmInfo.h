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

#ifndef LM32TARGETASMINFO_H
#define LM32TARGETASMINFO_H

#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCAsmInfo.h"

namespace llvm {
  class Target;

  class LM32MCAsmInfo : public MCAsmInfo {
  public:
    explicit LM32MCAsmInfo(const Target &T, StringRef TT);
  };


} // namespace llvm

#endif
