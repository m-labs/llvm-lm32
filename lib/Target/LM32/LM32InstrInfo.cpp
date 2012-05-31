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
#include "Mico32MachineFunctionInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define GET_INSTRINFO_CTOR
#include "Mico32GenInstrInfo.inc"

using namespace llvm;

Mico32InstrInfo::Mico32InstrInfo(Mico32TargetMachine &tm)
  : Mico32GenInstrInfo(Mico32::ADJCALLSTACKDOWN, Mico32::ADJCALLSTACKUP),
    TM(tm), RI(*TM.getSubtargetImpl(), *this) {}

#if 0
static bool isZeroImm(const MachineOperand &op) {
  return op.isImm() && op.getImm() == 0;
}
#endif 

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
#endif

void Mico32InstrInfo::
copyPhysReg(MachineBasicBlock &MBB,
            MachineBasicBlock::iterator I, DebugLoc DL,
            unsigned DestReg, unsigned SrcReg,
            bool KillSrc) const {
  llvm::BuildMI(MBB, I, DL, get(Mico32::ADD), DestReg)
    .addReg(SrcReg, getKillRegState(KillSrc)).addReg(Mico32::R0);
}


void Mico32InstrInfo::
storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                    unsigned SrcReg, bool isKill, int FI,
                    const TargetRegisterClass *RC,
                    const TargetRegisterInfo *TRI) const {
  assert(RC == Mico32::GPRRegisterClass && "Unknown register class.");
  DebugLoc DL;
  if (I != MBB.end()) DL = I->getDebugLoc();

  BuildMI(MBB, I, DL, get(Mico32::SW)).addReg(SrcReg,getKillRegState(isKill))
    .addFrameIndex(FI).addImm(0); 
}

void Mico32InstrInfo::
loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                     unsigned DestReg, int FI,
                     const TargetRegisterClass *RC,
                     const TargetRegisterInfo *TRI) const {
  assert(RC == Mico32::GPRRegisterClass && "Unexpected register class.");
  DebugLoc DL;
  if (I != MBB.end()) DL = I->getDebugLoc();

  BuildMI(MBB, I, DL, get(Mico32::LW), DestReg).addFrameIndex(FI).addImm(0);
}

//===----------------------------------------------------------------------===//
// Branch Analysis
//===----------------------------------------------------------------------===//

// Modeled from PTX:
MachineBasicBlock *Mico32InstrInfo::
GetBranchTarget(const MachineInstr& inst) {
  // The last explicit operand is the destiantion MBB in Mico32 branches.
  int NumOp = inst.getNumExplicitOperands();
  const MachineOperand& target = inst.getOperand(NumOp-1);

//  // FIXME So far all branch instructions put destination in 1st operand
//  const MachineOperand& target = inst.getOperand(0);
  assert(target.isMBB() && "FIXME: detect branch target operand");
  return target.getMBB();
}

// AnalyzeBranch returns a Boolean value and takes four parameters:
//    MachineBasicBlock &MBB — The incoming block to be examined.
//    MachineBasicBlock *&TBB — A destination block that is returned. For
//         a conditional branch that evaluates to true, TBB is the destination.
//    MachineBasicBlock *&FBB — For a conditional branch that evaluates to
//         false, FBB is returned as the destination.
//    std::vector<MachineOperand> &Cond — List of operands to evaluate a
//         condition for a conditional branch.
//  Return false if the branch is sucessfully analyzed, true otherwise.
//
//  Note that Cond appears to be  only used by the target specific code.  The
//  generic code just checks if Cond is empty or not.
bool Mico32InstrInfo::
AnalyzeBranch(MachineBasicBlock &MBB,
              MachineBasicBlock *&TBB,
              MachineBasicBlock *&FBB,
              SmallVectorImpl<MachineOperand> &Cond,
              bool AllowModify) const
{
// The Cond parameters need to be fixed - disable this for now.
return true;
    DEBUG(dbgs() << "AnalyzeBranch: blocks:\n");
    DEBUG(MBB.dump());
    DEBUG(dbgs() << "AnalyzeBranch: end blocks:\n");

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
  if (!isUnpredicatedTerminator(I)) {
    DEBUG(dbgs() << "AnalyzeBranch:predicated Terminator\n");
    DEBUG(dbgs() << "AnalyzeBranch: MBB:    " << MBB.getName().str() << "\n");
    return false;
  }

  // Get the last instruction in the block.
  MachineInstr *LastInst = I;

  // If there is only one terminator instruction, process it.
  unsigned LastOpc = LastInst->getOpcode();
  if (I == MBB.begin() || !isUnpredicatedTerminator(--I)) {
    if (Mico32::isUncondBranchOpcode(LastOpc)) {
  DEBUG(dbgs() << "AnalyzeBranch:one terminator instruction\n");
      TBB = LastInst->getOperand(0).getMBB();
  DEBUG(dbgs() << "AnalyzeBranch: instruction: " << *LastInst << "\n");
  DEBUG(dbgs() << "AnalyzeBranch: opcode: " << LastInst->getOpcode() << "\n");
  DEBUG(dbgs() << "AnalyzeBranch: MBB:    " << MBB.getName().str() << "\n");
  DEBUG(dbgs() << "AnalyzeBranch: TBB:    " << TBB->getName().str() << "\n");

      return false;
    }
    if (Mico32::isCondBranchOpcode(LastOpc)) {
      // Block ends with fall-through condbranch.
  DEBUG(dbgs() << "AnalyzeBranch:Block ends with fall-through condbranch\n");
  DEBUG(dbgs() << "AnalyzeBranch: instruction: " << *LastInst << "\n");
  DEBUG(dbgs() << "AnalyzeBranch: opcode: " << LastOpc << "\n");
  DEBUG(dbgs() << "AnalyzeBranch: MBB:    " << MBB.getName().str() << "\n");
      TBB = GetBranchTarget(*LastInst);
      Cond.push_back(MachineOperand::CreateImm(LastInst->getOpcode()));
      Cond.push_back(LastInst->getOperand(0));
  DEBUG(dbgs() << "AnalyzeBranch: TBB:    " << TBB->getName().str() << "\n");
  DEBUG(dbgs() << "AnalyzeBranch: Cond[0]:" << Cond[0] << "\n");
  DEBUG(dbgs() << "AnalyzeBranch: Cond[1]:" << Cond[1] << "\n");

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

  // The block ends with both a conditional branch and an ensuing
  // unconditional branch. 
  if (Mico32::isCondBranchOpcode(SecondLastInst->getOpcode()) &&
      Mico32::isUncondBranchOpcode(LastInst->getOpcode())) {
  DEBUG(dbgs() << "AnalyzeBranch:Conditional branch and ensuing unconditional\n");
    TBB = GetBranchTarget(*SecondLastInst);
    Cond.push_back(MachineOperand::CreateImm(SecondLastInst->getOpcode()));
    Cond.push_back(SecondLastInst->getOperand(0));
    FBB = GetBranchTarget(*LastInst);
  DEBUG(dbgs() << "AnalyzeBranch: 2nd instr:   " << *SecondLastInst << "\n");
  DEBUG(dbgs() << "AnalyzeBranch: last instr:  " << *LastInst << "\n");
  DEBUG(dbgs() << "AnalyzeBranch: 2nd opcode:  " << SecondLastInst->getOpcode() << "\n");
  DEBUG(dbgs() << "AnalyzeBranch: last opcode: " << LastInst->getOpcode() << "\n");
  DEBUG(dbgs() << "AnalyzeBranch: MBB:    " << MBB.getName().str() << "\n");
  DEBUG(dbgs() << "AnalyzeBranch: TBB:    " << TBB->getName().str() << "\n");
  DEBUG(dbgs() << "AnalyzeBranch: FBB:    " << FBB->getName().str() << "\n");
  DEBUG(dbgs() << "AnalyzeBranch: Cond[0]:" << Cond[0] << "\n");
  DEBUG(dbgs() << "AnalyzeBranch: Cond[1]:" << Cond[1] << "\n");

    return false;
  }

  // If the block ends with two unconditional branches, handle it.
  // The second one is not executed, so remove it.
  if (Mico32::isUncondBranchOpcode(SecondLastInst->getOpcode()) &&
      Mico32::isUncondBranchOpcode(LastInst->getOpcode())) {
    TBB = GetBranchTarget(*SecondLastInst);
    I = LastInst;
    if (AllowModify)
      I->eraseFromParent();
  DEBUG(dbgs() << "AnalyzeBranch:two unconditional branches\n");
  DEBUG(dbgs() << "AnalyzeBranch: instruction: " << *LastInst << "\n");
  DEBUG(dbgs() << "AnalyzeBranch: 2nd instr:   " << *SecondLastInst << "\n");
  DEBUG(dbgs() << "AnalyzeBranch: opcode: " << LastInst->getOpcode() << "\n");
  DEBUG(dbgs() << "AnalyzeBranch: 2nd opcode: " << SecondLastInst->getOpcode() << "\n");
  DEBUG(dbgs() << "AnalyzeBranch: MBB:    " << MBB.getName().str() << "\n");
  DEBUG(dbgs() << "AnalyzeBranch: TBB:    " << TBB->getName().str() << "\n");

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

  unsigned Opc = Mico32::BI;
  if (!Cond.empty())
    Opc = (unsigned)Cond[0].getImm();

  DEBUG(dbgs() << "InsertBranch:Opcode:  " << Opc << "\n");
  DEBUG(dbgs() << "InsertBranch: MBB:    " << MBB.getName().str() << "\n");
  DEBUG(dbgs() << "InsertBranch: MBB:    " << MBB << "\n");
  DEBUG( MBB.dump(););

  if (FBB == 0) {
    if (Cond.empty()) // Unconditional branch
      BuildMI(&MBB, DL, get(Opc)).addMBB(TBB);
    else              // Conditional branch
      BuildMI(&MBB, DL, get(Opc)).addReg(Cond[1].getReg()).addMBB(TBB);
    return 1;
  }

  BuildMI(&MBB, DL, get(Opc)).addReg(Cond[1].getReg()).addMBB(TBB);
  BuildMI(&MBB, DL, get(Mico32::BI)).addMBB(FBB);
  return 2;
}

unsigned Mico32InstrInfo::
RemoveBranch(MachineBasicBlock &MBB) const {
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

// Mico32 has be, bne, bg, bge, bgu, bgeu but not reverse conditions.  Can only
// reverse be/bne.
bool Mico32InstrInfo::
ReverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const {
  assert(Cond.size() == 2 && "Invalid Mico32 branch opcode!");
  DEBUG(dbgs() << "ReverseBranchCondition: Cond: " << Cond[0].getImm() << "\n");

  switch (Cond[0].getImm()) {
    default:            return true;
    case Mico32::BE:    Cond[0].setImm(Mico32::BNE); return false;
    case Mico32::BNE:   Cond[0].setImm(Mico32::BE); return false;
  }
}


#if 0
/// getGlobalBaseReg - Return a virtual register initialized with the
/// the global base register value. Output instructions required to
/// initialize the register in the function entry block, if necessary.
/// getGlobalBaseReg - insert code into the entry mbb to materialize the PIC
/// base register.  Return the virtual register that holds this value.
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
#endif
