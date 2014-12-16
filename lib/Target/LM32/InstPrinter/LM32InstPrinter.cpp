//===--- LM32InstPrinter.cpp - Convert LM32 MCInst to assembly syntax -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM instructions to GAS-format LM32 assembly language
// instructions.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "asm-printer"

#include "LM32InstPrinter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

//#define GET_INSTRUCTION_NAME
//#define PRINT_ALIAS_INSTR
#include "LM32GenAsmWriter.inc"

void LM32InstPrinter::
printInst(const MCInst *MI, raw_ostream &O, StringRef Annot) {
  printInstruction(MI, O);
  printAnnotation(O, Annot);
}

void LM32InstPrinter::
printRegName(raw_ostream &O, unsigned RegNo) const {
  O << '%' << getRegisterName(RegNo);
}

void LM32InstPrinter::
printUnsignedImm(const MCInst *MI, int opNum, raw_ostream &O)
{
  const MCOperand &MO = MI->getOperand(opNum);
  if (MO.isImm())
    O << (uint32_t)MO.getImm();
  else
    printOperand(MI, opNum, O);
}

// Used for ADDRri addressing mode.
// This will get passed a register and offset.
// Modifiers are specified in the XXXInstrInfo.td file.  E.g. declaring
// the instruction as 
//  <..."add ${addr:arith}, $dst", [(set IntRegs:$dst, ADDRri:$addr)..>;
// will cause TableGen to pass the modifier "arith" to printMemOperand.
// See SPARC for example.
void LM32InstPrinter::
printMemOperand(const MCInst *MI, int opNum, raw_ostream &O)
{
  O << "(";
  printOperand(MI, opNum, O);
  O << "+";
  printOperand(MI, opNum+1, O);
  O << ")";
}

void LM32InstPrinter::
printOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O) const
{
    const MCOperand& MO = MI->getOperand(OpNo);
    
    if (MO.isReg()) {
        O << getRegisterName(MO.getReg());
    } else if(MO.isExpr()) {
        O << *MO.getExpr();
    } else if(MO.isImm()) {
        printImmOperand(MI, OpNo, O);
    } else {
        llvm_unreachable("Unknown operand");
    }
}


void LM32InstPrinter::
printImmOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O) const
{
    const MCOperand& MO = MI->getOperand(OpNo);
    
    if(MO.isExpr()) {
        O << *MO.getExpr();
    } else if(MO.isImm()) {
        O << MI->getOperand(OpNo).getImm();
    } else {
        llvm_unreachable("Unknown operand");
    }
}


void LM32InstPrinter::
printS16ImmOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O)
{
  O << (short) MI->getOperand(OpNo).getImm();
}

void LM32InstPrinter::
printSymbol(const MCInst *MI, unsigned OpNo,raw_ostream &O, bool hi) const
{
    assert((MI->getOperand(OpNo).isImm() || MI->getOperand(OpNo).isExpr())
           && "Unknown symbol operand");
    
    O << (hi ? "hi" : "lo") << "(";
    printOperand(MI, OpNo, O);
    O << ')';
}

