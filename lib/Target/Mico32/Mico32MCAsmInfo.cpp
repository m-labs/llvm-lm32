//===-- Mico32MCAsmInfo.cpp - Mico32 asm properties -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the Mico32MCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "Mico32MCAsmInfo.h"
using namespace llvm;

Mico32MCAsmInfo::Mico32MCAsmInfo() {
  SupportsDebugInformation    = true;
  AlignmentIsInBytes          = false;
  PrivateGlobalPrefix         = "$";
  GPRel32Directive            = "\t.gpword\t";
}
