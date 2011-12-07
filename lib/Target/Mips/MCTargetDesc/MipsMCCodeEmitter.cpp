//===-- MipsMCCodeEmitter.cpp - Convert Mips code to machine code ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the MipsMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//
//
#define DEBUG_TYPE "mccodeemitter"
#include "MCTargetDesc/MipsBaseInfo.h"
#include "MCTargetDesc/MipsFixupKinds.h"
#include "MCTargetDesc/MipsMCTargetDesc.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
class MipsMCCodeEmitter : public MCCodeEmitter {
  MipsMCCodeEmitter(const MipsMCCodeEmitter &); // DO NOT IMPLEMENT
  void operator=(const MipsMCCodeEmitter &); // DO NOT IMPLEMENT
  const MCInstrInfo &MCII;
  const MCSubtargetInfo &STI;
  MCContext &Ctx;

public:
  MipsMCCodeEmitter(const MCInstrInfo &mcii, const MCSubtargetInfo &sti,
                    MCContext &ctx) : MCII(mcii), STI(sti) , Ctx(ctx) {}

  ~MipsMCCodeEmitter() {}

  void EmitByte(unsigned char C, raw_ostream &OS) const {
    OS << (char)C;
  }

  void EmitInstruction(uint64_t Val, unsigned Size, raw_ostream &OS) const {
    // Output the instruction encoding in little endian byte order.
    for (unsigned i = 0; i != Size; ++i) {
      EmitByte(Val & 255, OS);
      Val >>= 8;
    }
  }

  void EncodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups) const;

  // getBinaryCodeForInstr - TableGen'erated function for getting the
  // binary encoding for an instruction.
  unsigned getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups) const;

  // getBranchJumpOpValue - Return binary encoding of the jump
  // target operand. If the machine operand requires relocation,
  // record the relocation and return zero.
   unsigned getJumpTargetOpValue(const MCInst &MI, unsigned OpNo,
                                 SmallVectorImpl<MCFixup> &Fixups) const;

   // getBranchTargetOpValue - Return binary encoding of the branch
   // target operand. If the machine operand requires relocation,
   // record the relocation and return zero.
  unsigned getBranchTargetOpValue(const MCInst &MI, unsigned OpNo,
                                  SmallVectorImpl<MCFixup> &Fixups) const;

   // getMachineOpValue - Return binary encoding of operand. If the machin
   // operand requires relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI,const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups) const;

  unsigned getMemEncoding(const MCInst &MI, unsigned OpNo,
                          SmallVectorImpl<MCFixup> &Fixups) const;
  unsigned getSizeExtEncoding(const MCInst &MI, unsigned OpNo,
                              SmallVectorImpl<MCFixup> &Fixups) const;
  unsigned getSizeInsEncoding(const MCInst &MI, unsigned OpNo,
                              SmallVectorImpl<MCFixup> &Fixups) const;

}; // class MipsMCCodeEmitter
}  // namespace

MCCodeEmitter *llvm::createMipsMCCodeEmitter(const MCInstrInfo &MCII,
                                             const MCSubtargetInfo &STI,
                                             MCContext &Ctx)
{
  return new MipsMCCodeEmitter(MCII, STI, Ctx);
}

/// EncodeInstruction - Emit the instruction.
/// Size the instruction (currently only 4 bytes
void MipsMCCodeEmitter::
EncodeInstruction(const MCInst &MI, raw_ostream &OS,
                  SmallVectorImpl<MCFixup> &Fixups) const
{
  uint32_t Binary = getBinaryCodeForInstr(MI, Fixups);

  // Check for unimplemented opcodes.
  // Unfortunately in MIPS both NOT and SLL will come in with Binary == 0
  // so we have to special check for them.
  unsigned Opcode = MI.getOpcode();
  if ((Opcode != Mips::NOP) && (Opcode != Mips::SLL) && !Binary)
    llvm_unreachable("unimplemented opcode in EncodeInstruction()");

  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  uint64_t TSFlags = Desc.TSFlags;

  // Pseudo instructions don't get encoded and shouldn't be here
  // in the first place!
  if ((TSFlags & MipsII::FormMask) == MipsII::Pseudo)
    llvm_unreachable("Pseudo opcode found in EncodeInstruction()");

  // For now all instructions are 4 bytes
  int Size = 4; // FIXME: Have Desc.getSize() return the correct value!

  EmitInstruction(Binary, Size, OS);
}

/// getBranchTargetOpValue - Return binary encoding of the branch
/// target operand. If the machine operand requires relocation,
/// record the relocation and return zero.
unsigned MipsMCCodeEmitter::
getBranchTargetOpValue(const MCInst &MI, unsigned OpNo,
                       SmallVectorImpl<MCFixup> &Fixups) const {

  const MCOperand &MO = MI.getOperand(OpNo);
  assert(MO.isExpr() && "getBranchTargetOpValue expects only expressions");

  const MCExpr *Expr = MO.getExpr();
  Fixups.push_back(MCFixup::Create(0, Expr,
                                   MCFixupKind(Mips::fixup_Mips_PC16)));
  return 0;
}

/// getJumpTargetOpValue - Return binary encoding of the jump
/// target operand. If the machine operand requires relocation,
/// record the relocation and return zero.
unsigned MipsMCCodeEmitter::
getJumpTargetOpValue(const MCInst &MI, unsigned OpNo,
                     SmallVectorImpl<MCFixup> &Fixups) const {

  const MCOperand &MO = MI.getOperand(OpNo);
  assert(MO.isExpr() && "getJumpTargetOpValue expects only expressions");

  const MCExpr *Expr = MO.getExpr();
  Fixups.push_back(MCFixup::Create(0, Expr,
                                   MCFixupKind(Mips::fixup_Mips_26)));
  return 0;
}

/// getMachineOpValue - Return binary encoding of operand. If the machine
/// operand requires relocation, record the relocation and return zero.
unsigned MipsMCCodeEmitter::
getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                  SmallVectorImpl<MCFixup> &Fixups) const {
  if (MO.isReg()) {
    unsigned Reg = MO.getReg();
    unsigned RegNo = getMipsRegisterNumbering(Reg);
    return RegNo;
  } else if (MO.isImm()) {
    return static_cast<unsigned>(MO.getImm());
  } else if (MO.isFPImm()) {
    return static_cast<unsigned>(APFloat(MO.getFPImm())
        .bitcastToAPInt().getHiBits(32).getLimitedValue());
  } else if (MO.isExpr()) {
    const MCExpr *Expr = MO.getExpr();
    MCExpr::ExprKind Kind = Expr->getKind();
    unsigned Ret = 0;

    if (Kind == MCExpr::Binary) {
      const MCBinaryExpr *BE = static_cast<const MCBinaryExpr*>(Expr);
      Expr = BE->getLHS();
      Kind = Expr->getKind();
      const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(BE->getRHS());
      assert((Kind == MCExpr::SymbolRef) && CE &&
             "Binary expression must be sym+const.");
      Ret = CE->getValue();
    }

    if (Kind == MCExpr::SymbolRef) {
      Mips::Fixups FixupKind;
      switch(cast<MCSymbolRefExpr>(Expr)->getKind()) {
      case MCSymbolRefExpr::VK_Mips_GPREL:
        FixupKind = Mips::fixup_Mips_GPREL16;
        break;
      case MCSymbolRefExpr::VK_Mips_GOT_CALL:
        FixupKind = Mips::fixup_Mips_CALL16;
        break;
      case MCSymbolRefExpr::VK_Mips_GOT16:
        FixupKind = Mips::fixup_Mips_GOT_Global;
        break;
      case MCSymbolRefExpr::VK_Mips_GOT:
        FixupKind = Mips::fixup_Mips_GOT_Local;
        break;
      case MCSymbolRefExpr::VK_Mips_ABS_HI:
        FixupKind = Mips::fixup_Mips_HI16;
        break;
      case MCSymbolRefExpr::VK_Mips_ABS_LO:
        FixupKind = Mips::fixup_Mips_LO16;
        break;
      case MCSymbolRefExpr::VK_Mips_TLSGD:
        FixupKind = Mips::fixup_Mips_TLSGD;
        break;
      case MCSymbolRefExpr::VK_Mips_GOTTPREL:
        FixupKind = Mips::fixup_Mips_GOTTPREL;
        break;
      case MCSymbolRefExpr::VK_Mips_TPREL_HI:
        FixupKind = Mips::fixup_Mips_TPREL_HI;
        break;
      case MCSymbolRefExpr::VK_Mips_TPREL_LO:
        FixupKind = Mips::fixup_Mips_TPREL_LO;
        break;
      default:
        return Ret;
      } // switch
      Fixups.push_back(MCFixup::Create(0, Expr, MCFixupKind(FixupKind)));
    } // if SymbolRef
    // All of the information is in the fixup.
    return Ret;
  }
  llvm_unreachable("Unable to encode MCOperand!");
  // Not reached
  return 0;
}

/// getMemEncoding - Return binary encoding of memory related operand.
/// If the offset operand requires relocation, record the relocation.
unsigned
MipsMCCodeEmitter::getMemEncoding(const MCInst &MI, unsigned OpNo,
                                  SmallVectorImpl<MCFixup> &Fixups) const {
  // Base register is encoded in bits 20-16, offset is encoded in bits 15-0.
  assert(MI.getOperand(OpNo).isReg());
  unsigned RegBits = getMachineOpValue(MI, MI.getOperand(OpNo),Fixups) << 16;
  unsigned OffBits = getMachineOpValue(MI, MI.getOperand(OpNo+1), Fixups);

  return (OffBits & 0xFFFF) | RegBits;
}

unsigned
MipsMCCodeEmitter::getSizeExtEncoding(const MCInst &MI, unsigned OpNo,
                                      SmallVectorImpl<MCFixup> &Fixups) const {
  assert(MI.getOperand(OpNo).isImm());
  unsigned szEncoding = getMachineOpValue(MI, MI.getOperand(OpNo), Fixups);
  return szEncoding - 1;
}

// FIXME: should be called getMSBEncoding
//
unsigned
MipsMCCodeEmitter::getSizeInsEncoding(const MCInst &MI, unsigned OpNo,
                                      SmallVectorImpl<MCFixup> &Fixups) const {
  assert(MI.getOperand(OpNo-1).isImm());
  assert(MI.getOperand(OpNo).isImm());
  unsigned pos = getMachineOpValue(MI, MI.getOperand(OpNo-1), Fixups);
  unsigned sz = getMachineOpValue(MI, MI.getOperand(OpNo), Fixups);

  return pos + sz - 1;
}

#include "MipsGenMCCodeEmitter.inc"

