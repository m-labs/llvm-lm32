
//===-- LM32AsmPrinter.h - LM32 LLVM assembly printer ----*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LM32_LM32ASMPRINTER_H
#define LLVM_LIB_TARGET_LM32_LM32ASMPRINTER_H

#include "LM32TargetMachine.h"
#include "LM32MCInstLower.h"
#include "llvm/CodeGen/AsmPrinter.h"

namespace llvm {
class MCStreamer;
class MachineBasicBlock;
class MachineInstr;
class Module;
class raw_ostream;

class LLVM_LIBRARY_VISIBILITY LM32AsmPrinter : public AsmPrinter {
private:
  const LM32Subtarget *Subtarget;
  LM32MCInstLower MCInstLowering;

public:
  explicit LM32AsmPrinter(TargetMachine &TM, MCStreamer &Streamer)
    : AsmPrinter(TM, Streamer),
      Subtarget(&TM.getSubtarget<LM32Subtarget>()), MCInstLowering(*this) {}

  const char *getPassName() const override {
    return "LM32 Assembly Printer";
  }

/*
  static const char *getRegisterName(unsigned RegNo) {
    return LM32InstPrinter::getRegisterName(RegNo);
  }
*/
  void EmitInstruction(const MachineInstr *MI) override;
/*
  void EmitMachineConstantPoolValue(MachineConstantPoolValue *MCPV) override;
  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       unsigned AsmVariant, const char *ExtraCode,
                       raw_ostream &OS) override;
  bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                             unsigned AsmVariant, const char *ExtraCode,
                             raw_ostream &OS) override;
  void EmitEndOfAsmFile(Module &M) override;
*/
};
} // end namespace llvm

#endif  // LLVM_LIB_TARGET_LM32_LM32ASMPRINTER_H
