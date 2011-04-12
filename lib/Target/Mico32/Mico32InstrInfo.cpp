//===- Mico32InstrInfo.cpp - Mico32 Instruction Information -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Mico32 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "Mico32InstrInfo.h"
#include "Mico32TargetMachine.h"
#include "Mico32MachineFunction.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "Mico32GenInstrInfo.inc"

using namespace llvm;

Mico32InstrInfo::Mico32InstrInfo(Mico32TargetMachine &tm)
  : TargetInstrInfoImpl(Mico32Insts, array_lengthof(Mico32Insts)),
    TM(tm), RI(*TM.getSubtargetImpl(), *this) {}

static bool isZeroImm(const MachineOperand &op) {
  return op.isImm() && op.getImm() == 0;
}
#if 0

/// isLoadFromStackSlot - If the specified machine instruction is a direct
/// load from a stack slot, return the virtual or physical register number of
/// the destination along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than loading from the stack slot.
unsigned Mico32InstrInfo::
isLoadFromStackSlot(const MachineInstr *MI, int &FrameIndex) const {
  if (MI->getOpcode() == Mico32::LWI) {
    if ((MI->getOperand(1).isFI()) && // is a stack slot
        (MI->getOperand(2).isImm()) &&  // the imm is zero
        (isZeroImm(MI->getOperand(2)))) {
      FrameIndex = MI->getOperand(1).getIndex();
      return MI->getOperand(0).getReg();
    }
  }

  return 0;
}

/// isStoreToStackSlot - If the specified machine instruction is a direct
/// store to a stack slot, return the virtual or physical register number of
/// the source reg along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than storing to the stack slot.
unsigned Mico32InstrInfo::
isStoreToStackSlot(const MachineInstr *MI, int &FrameIndex) const {
  if (MI->getOpcode() == Mico32::SWI) {
    if ((MI->getOperand(1).isFI()) && // is a stack slot
        (MI->getOperand(2).isImm()) &&  // the imm is zero
        (isZeroImm(MI->getOperand(2)))) {
      FrameIndex = MI->getOperand(1).getIndex();
      return MI->getOperand(0).getReg();
    }
  }
  return 0;
}

/// insertNoop - If data hazard condition is found insert the target nop
/// instruction.
void Mico32InstrInfo::
insertNoop(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI) const {
  DebugLoc DL;
  BuildMI(MBB, MI, DL, get(Mico32::NOP));
}

void Mico32InstrInfo::
copyPhysReg(MachineBasicBlock &MBB,
            MachineBasicBlock::iterator I, DebugLoc DL,
            unsigned DestReg, unsigned SrcReg,
            bool KillSrc) const {
  llvm::BuildMI(MBB, I, DL, get(Mico32::ADDK), DestReg)
    .addReg(SrcReg, getKillRegState(KillSrc)).addReg(Mico32::R0);
}

void Mico32InstrInfo::
storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                    unsigned SrcReg, bool isKill, int FI,
                    const TargetRegisterClass *RC,
                    const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  BuildMI(MBB, I, DL, get(Mico32::SWI)).addReg(SrcReg,getKillRegState(isKill))
    .addFrameIndex(FI).addImm(0); //.addFrameIndex(FI);
}

void Mico32InstrInfo::
loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                     unsigned DestReg, int FI,
                     const TargetRegisterClass *RC,
                     const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  BuildMI(MBB, I, DL, get(Mico32::LWI), DestReg)
      .addFrameIndex(FI).addImm(0); //.addFrameIndex(FI);
}

//===----------------------------------------------------------------------===//
// Branch Analysis
//===----------------------------------------------------------------------===//
bool Mico32InstrInfo::AnalyzeBranch(MachineBasicBlock &MBB,
                                    MachineBasicBlock *&TBB,
                                    MachineBasicBlock *&FBB,
                                    SmallVectorImpl<MachineOperand> &Cond,
                                    bool AllowModify) const {
  // If the block has no terminators, it just falls into the block after it.
  MachineBasicBlock::iterator I = MBB.end();
  if (I == MBB.begin())
    return false;
  --I;
  while (I->isDebugValue()) {
    if (I == MBB.begin())
      return false;
    --I;
  }
  if (!isUnpredicatedTerminator(I))
    return false;

  // Get the last instruction in the block.
  MachineInstr *LastInst = I;

  // If there is only one terminator instruction, process it.
  unsigned LastOpc = LastInst->getOpcode();
  if (I == MBB.begin() || !isUnpredicatedTerminator(--I)) {
    if (Mico32::isUncondBranchOpcode(LastOpc)) {
      TBB = LastInst->getOperand(0).getMBB();
      return false;
    }
    if (Mico32::isCondBranchOpcode(LastOpc)) {
      // Block ends with fall-through condbranch.
      TBB = LastInst->getOperand(1).getMBB();
      Cond.push_back(MachineOperand::CreateImm(LastInst->getOpcode()));
      Cond.push_back(LastInst->getOperand(0));
      return false;
    }
    // Otherwise, don't know what this is.
    return true;
  }

  // Get the instruction before it if it's a terminator.
  MachineInstr *SecondLastInst = I;

  // If there are three terminators, we don't know what sort of block this is.
  if (SecondLastInst && I != MBB.begin() && isUnpredicatedTerminator(--I))
    return true;

  // If the block ends with something like BEQID then BRID, handle it.
  if (Mico32::isCondBranchOpcode(SecondLastInst->getOpcode()) &&
      Mico32::isUncondBranchOpcode(LastInst->getOpcode())) {
    TBB = SecondLastInst->getOperand(1).getMBB();
    Cond.push_back(MachineOperand::CreateImm(SecondLastInst->getOpcode()));
    Cond.push_back(SecondLastInst->getOperand(0));
    FBB = LastInst->getOperand(0).getMBB();
    return false;
  }

  // If the block ends with two unconditional branches, handle it.
  // The second one is not executed, so remove it.
  if (Mico32::isUncondBranchOpcode(SecondLastInst->getOpcode()) &&
      Mico32::isUncondBranchOpcode(LastInst->getOpcode())) {
    TBB = SecondLastInst->getOperand(0).getMBB();
    I = LastInst;
    if (AllowModify)
      I->eraseFromParent();
    return false;
  }

  // Otherwise, can't handle this.
  return true;
}

unsigned Mico32InstrInfo::
InsertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
             MachineBasicBlock *FBB,
             const SmallVectorImpl<MachineOperand> &Cond,
             DebugLoc DL) const {
  // Shouldn't be a fall through.
  assert(TBB && "InsertBranch must not be told to insert a fallthrough");
  assert((Cond.size() == 2 || Cond.size() == 0) &&
         "Mico32 branch conditions have two components!");

  unsigned Opc = Mico32::BRID;
  if (!Cond.empty())
    Opc = (unsigned)Cond[0].getImm();

  if (FBB == 0) {
    if (Cond.empty()) // Unconditional branch
      BuildMI(&MBB, DL, get(Opc)).addMBB(TBB);
    else              // Conditional branch
      BuildMI(&MBB, DL, get(Opc)).addReg(Cond[1].getReg()).addMBB(TBB);
    return 1;
  }

  BuildMI(&MBB, DL, get(Opc)).addReg(Cond[1].getReg()).addMBB(TBB);
  BuildMI(&MBB, DL, get(Mico32::BRID)).addMBB(FBB);
  return 2;
}

unsigned Mico32InstrInfo::RemoveBranch(MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator I = MBB.end();
  if (I == MBB.begin()) return 0;
  --I;
  while (I->isDebugValue()) {
    if (I == MBB.begin())
      return 0;
    --I;
  }

  if (!Mico32::isUncondBranchOpcode(I->getOpcode()) &&
      !Mico32::isCondBranchOpcode(I->getOpcode()))
    return 0;

  // Remove the branch.
  I->eraseFromParent();

  I = MBB.end();

  if (I == MBB.begin()) return 1;
  --I;
  if (!Mico32::isCondBranchOpcode(I->getOpcode()))
    return 1;

  // Remove the branch.
  I->eraseFromParent();
  return 2;
}

bool Mico32InstrInfo::ReverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const {
  assert(Cond.size() == 2 && "Invalid Mico32 branch opcode!");
  switch (Cond[0].getImm()) {
  default:            return true;
  case Mico32::BEQ:   Cond[0].setImm(Mico32::BNE); return false;
  case Mico32::BNE:   Cond[0].setImm(Mico32::BEQ); return false;
  case Mico32::BGT:   Cond[0].setImm(Mico32::BLE); return false;
  case Mico32::BGE:   Cond[0].setImm(Mico32::BLT); return false;
  case Mico32::BLT:   Cond[0].setImm(Mico32::BGE); return false;
  case Mico32::BLE:   Cond[0].setImm(Mico32::BGT); return false;
  case Mico32::BEQI:  Cond[0].setImm(Mico32::BNEI); return false;
  case Mico32::BNEI:  Cond[0].setImm(Mico32::BEQI); return false;
  case Mico32::BGTI:  Cond[0].setImm(Mico32::BLEI); return false;
  case Mico32::BGEI:  Cond[0].setImm(Mico32::BLTI); return false;
  case Mico32::BLTI:  Cond[0].setImm(Mico32::BGEI); return false;
  case Mico32::BLEI:  Cond[0].setImm(Mico32::BGTI); return false;
  case Mico32::BEQD:  Cond[0].setImm(Mico32::BNED); return false;
  case Mico32::BNED:  Cond[0].setImm(Mico32::BEQD); return false;
  case Mico32::BGTD:  Cond[0].setImm(Mico32::BLED); return false;
  case Mico32::BGED:  Cond[0].setImm(Mico32::BLTD); return false;
  case Mico32::BLTD:  Cond[0].setImm(Mico32::BGED); return false;
  case Mico32::BLED:  Cond[0].setImm(Mico32::BGTD); return false;
  case Mico32::BEQID: Cond[0].setImm(Mico32::BNEID); return false;
  case Mico32::BNEID: Cond[0].setImm(Mico32::BEQID); return false;
  case Mico32::BGTID: Cond[0].setImm(Mico32::BLEID); return false;
  case Mico32::BGEID: Cond[0].setImm(Mico32::BLTID); return false;
  case Mico32::BLTID: Cond[0].setImm(Mico32::BGEID); return false;
  case Mico32::BLEID: Cond[0].setImm(Mico32::BGTID); return false;
  }
}

#endif
/// getGlobalBaseReg - Return a virtual register initialized with the
/// the global base register value. Output instructions required to
/// initialize the register in the function entry block, if necessary.
///
unsigned Mico32InstrInfo::getGlobalBaseReg(MachineFunction *MF) const {
//FIXME: not ported to Mico32
  Mico32FunctionInfo *Mico32FI = MF->getInfo<Mico32FunctionInfo>();
  unsigned GlobalBaseReg = Mico32FI->getGlobalBaseReg();
  if (GlobalBaseReg != 0)
    return GlobalBaseReg;

  // Insert the set of GlobalBaseReg into the first MBB of the function
  MachineBasicBlock &FirstMBB = MF->front();
  MachineBasicBlock::iterator MBBI = FirstMBB.begin();
  MachineRegisterInfo &RegInfo = MF->getRegInfo();
  const TargetInstrInfo *TII = MF->getTarget().getInstrInfo();

  GlobalBaseReg = RegInfo.createVirtualRegister(Mico32::GPRRegisterClass);
  BuildMI(FirstMBB, MBBI, DebugLoc(), TII->get(TargetOpcode::COPY),
          GlobalBaseReg).addReg(Mico32::R20);
  RegInfo.addLiveIn(Mico32::R20);

  Mico32FI->setGlobalBaseReg(GlobalBaseReg);
  return GlobalBaseReg;
}
