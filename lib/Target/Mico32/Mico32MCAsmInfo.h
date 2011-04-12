//=====-- Mico32MCAsmInfo.h - Mico32 asm properties -----------*- C++ -*--====//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the Mico32MCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef MICO32TARGETASMINFO_H
#define MICO32TARGETASMINFO_H

#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCAsmInfo.h"

namespace llvm {
  class Target;

  class Mico32ELFMCAsmInfo : public MCAsmInfo {
  public:
    explicit Mico32ELFMCAsmInfo(const Target &T, const StringRef &TT);
  };

} // namespace llvm

#endif
