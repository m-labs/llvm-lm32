
//===-- LM32MCInstLower.h - Lower MachineInstr to MCInst ----*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LM32_LM32MCINSTLOWER_H
#define LLVM_LIB_TARGET_LM32_LM32MCINSTLOWER_H

#include "llvm/MC/MCExpr.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/DataTypes.h"

namespace llvm {
class MCInst;
class MCOperand;
class MachineInstr;
class MachineOperand;
class Mangler;
class LM32AsmPrinter;

///
/// from SystemZ
///

class LLVM_LIBRARY_VISIBILITY LM32MCInstLower {
  LM32AsmPrinter &AsmPrinter;
  const MCExpr * getExpr(const MachineOperand &MO) const;

public:
  LM32MCInstLower(LM32AsmPrinter &asmPrinter);

  // Lower MachineInstr MI to MCInst OutMI.
  void lower(const MachineInstr *MI, MCInst &OutMI) const;

  // Return an MCOperand for MO.
  MCOperand lowerOperand(const MachineOperand& MO) const;

};
} // end namespace llvm

#endif  // LLVM_LIB_TARGET_LM32_LM32MCINSTLOWER_H
