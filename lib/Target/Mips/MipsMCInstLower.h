//===-- MipsMCInstLower.h - Lower MachineInstr to MCInst -------------------==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef MIPSMCINSTLOWER_H
#define MIPSMCINSTLOWER_H
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/Support/Compiler.h"

namespace llvm {
  class MCAsmInfo;
  class MCContext;
  class MCInst;
  class MCOperand;
  class MCSymbol;
  class MachineInstr;
  class MachineFunction;
  class Mangler;
  class MipsAsmPrinter;
  
/// MipsMCInstLower - This class is used to lower an MachineInstr into an
//                    MCInst.
class LLVM_LIBRARY_VISIBILITY MipsMCInstLower {
  typedef MachineOperand::MachineOperandType MachineOperandType;
  MCContext &Ctx;
  Mangler *Mang;
  MipsAsmPrinter &AsmPrinter;
public:
  MipsMCInstLower(Mangler *mang, const MachineFunction &MF,
                  MipsAsmPrinter &asmprinter);  
  void Lower(const MachineInstr *MI, MCInst &OutMI) const;
  void LowerCPLOAD(const MachineInstr *MI, SmallVector<MCInst, 4>& MCInsts);
  void LowerCPRESTORE(const MachineInstr *MI, SmallVector<MCInst, 4>& MCInsts); 
  void LowerUnalignedLoadStore(const MachineInstr *MI,
		                           SmallVector<MCInst, 4>& MCInsts);
private:
  MCOperand LowerSymbolOperand(const MachineOperand &MO,
                               MachineOperandType MOTy, unsigned Offset) const;
  MCOperand LowerOperand(const MachineOperand& MO, unsigned offset = 0) const;
};
}

#endif
