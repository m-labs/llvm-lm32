//===-- LM32MCAsmInfo.cpp - LM32 asm properties ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the LM32MCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "LM32MCAsmInfo.h"

using namespace llvm;

void LM32MCAsmInfo::anchor() { }

LM32MCAsmInfo::LM32MCAsmInfo(StringRef TT)
{
  IsLittleEndian = false;
  SupportsDebugInformation    = true;
//  AlignmentIsInBytes          = false;
  AlignmentIsInBytes          = true;
  PrivateGlobalPrefix         = ".L";
  PrivateLabelPrefix          = ".L";
  WeakRefDirective            = "\t.weak\t";
  GPRel32Directive            = "\t.gpword\t";

  // Uses '.section' before '.bss' directive
  UsesELFSectionDirectiveForBSS = true;
}



