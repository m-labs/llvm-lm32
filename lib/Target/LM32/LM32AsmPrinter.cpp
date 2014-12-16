//===-- LM32AsmPrinter.cpp - LM32 LLVM assembly writer --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to GAS-format LM32 assembly language.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "lm32-asm-printer"

#include "LM32AsmPrinter.h"
#include "LM32MCInstLower.h"
#include "InstPrinter/LM32InstPrinter.h"
#include "LM32.h"
#include "LM32InstrInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Module.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/IR/Mangler.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

namespace {
/*  class LM32AsmPrinter : public AsmPrinter {
  public:
    explicit LM32AsmPrinter(TargetMachine &TM, MCStreamer &Streamer)
      : AsmPrinter(TM, Streamer) {}

 

    static const char *getRegisterName(unsigned RegNo) {
      return LM32InstPrinter::getRegisterName(RegNo);
    }

    void EmitInstruction(const MachineInstr *MI) override;
  private:
    LM32MCInstLower MCInstLowering;
    
/ *
    // for inline assembly printing
    void printOperand(const MachineInstr *MI, int opNum, raw_ostream &O);
    bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                         unsigned AsmVariant, const char *ExtraCode,
                         raw_ostream &O) override;
    bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                               unsigned AsmVariant, const char *ExtraCode,
                               raw_ostream &O) override;
 * /
  };
 */
} // end of anonymous namespace

// Force static initialization.
extern "C" void
LLVMInitializeLM32AsmPrinter() {
  RegisterAsmPrinter<LM32AsmPrinter> X(TheLM32Target);
}

/*
void LM32AsmPrinter::EmitInstruction(const MachineInstr *MI) {
      SmallString<128> Str;
      raw_svector_ostream OS(Str);
      printInstruction(MI, OS);
      OutStreamer.EmitRawText(OS.str());
}
*/
// See ARM/Mips
void LM32AsmPrinter::EmitInstruction(const MachineInstr *MI) {
    assert((&(MF->getContext()) == &(AsmPrinter::OutContext))  &&
           "we are treating these as the same in  LM32MCInstLower");
    // We have no auto-generated pseudo lowerings.
    if (MI->isPseudo())
        llvm_unreachable("Pseudo opcode found in EmitInstruction()");
    
    // In the future we'll need to check for manual lowerings.
    MCInst TmpInst0;
    MCInstLowering.lower(MI, TmpInst0);
    EmitToStreamer(OutStreamer, TmpInst0);
}

/*
// used only for inline asm expressions.
void LM32AsmPrinter::
printOperand(const MachineInstr *MI, int opNum, raw_ostream &O)
{
  const MachineOperand &MO = MI->getOperand(opNum);
  switch (MO.getType()) {
  case MachineOperand::MO_Register:
    assert(TargetRegisterInfo::isPhysicalRegister(MO.getReg()) &&
           "Virtual registers should be already mapped!");
    O << getRegisterName(MO.getReg());
    break;

  case MachineOperand::MO_Immediate:
    O << MO.getImm();
    break;
  case MachineOperand::MO_MachineBasicBlock:
    O << *MO.getMBB()->getSymbol();
    return;
  case MachineOperand::MO_GlobalAddress:
    O << *Mang->getSymbol(MO.getGlobal());
    printOffset(MO.getOffset(), O);
    break;
  case MachineOperand::MO_ExternalSymbol:
    O << *GetExternalSymbolSymbol(MO.getSymbolName());
    break;
  case MachineOperand::MO_ConstantPoolIndex:
    O << MAI->getPrivateGlobalPrefix() << "CPI" << getFunctionNumber() << "_"
      << MO.getIndex();
    break;
  case MachineOperand::MO_JumpTableIndex:
    O << MAI->getPrivateGlobalPrefix() << "JTI" << getFunctionNumber()
      << '_' << MO.getIndex();
    break;
  default:
    llvm_unreachable("<unknown operand type>");
    break;
  }
}
*/

/*
/// PrintAsmOperand - Print out an operand for an inline asm expression.
///
bool LM32AsmPrinter::
PrintAsmOperand(const MachineInstr *MI, unsigned OpNo, unsigned AsmVariant,
                const char *ExtraCode, raw_ostream &O)
{
  if (ExtraCode && ExtraCode[0]) {
    if (ExtraCode[1] != 0) return true; // Unknown modifier.

    switch (ExtraCode[0]) {
    default: return true;  // Unknown modifier.
    case 'r':
      break;
    }
  }

  printOperand(MI, OpNo, O);

  return false;
}
*/
/*
/// PrintAsmMemoryOperand - Print out a memory operand for an inline asm expression.
bool LM32AsmPrinter::
PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                      unsigned AsmVariant, const char *ExtraCode,
                      raw_ostream &O)
{
  if (ExtraCode && ExtraCode[0])
    return true;  // Unknown modifier

  O << '[';
  printOperand(MI, OpNo, O);
  O << ']';

  return false;
}
*/
