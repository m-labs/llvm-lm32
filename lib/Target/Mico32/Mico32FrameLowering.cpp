//===- Mico32FrameLowering.cpp - Mico32 Frame Information ------*- C++ -*-====//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Mico32 implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "mico32-frame-lowering"

#include "Mico32FrameLowering.h"
#include "Mico32InstrInfo.h"
#include "Mico32MachineFunction.h"
//#include "InstPrinter/Mico32InstPrinter.h"
#include "llvm/Function.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace llvm {
  cl::opt<bool> Mico32DisableStackAdjust(
    "disable-mico32-stack-adjust",
    cl::init(false),
    cl::desc("Disable Mico32 stack layout adjustment."),
    cl::Hidden);
}

static void replaceFrameIndexes(MachineFunction &MF, 
                                SmallVector<std::pair<int,int64_t>, 16> &FR) {
//FIXME: Not ported to Mico32
  MachineFrameInfo *MFI = MF.getFrameInfo();
  Mico32FunctionInfo *Mico32FI = MF.getInfo<Mico32FunctionInfo>();
  const SmallVector<std::pair<int,int64_t>, 16>::iterator FRB = FR.begin();
  const SmallVector<std::pair<int,int64_t>, 16>::iterator FRE = FR.end();

  SmallVector<std::pair<int,int64_t>, 16>::iterator FRI = FRB;
  for (; FRI != FRE; ++FRI) {
    MFI->RemoveStackObject(FRI->first);
    int NFI = MFI->CreateFixedObject(4, FRI->second, true);
    Mico32FI->recordReplacement(FRI->first, NFI);

    for (MachineFunction::iterator MB=MF.begin(), ME=MF.end(); MB!=ME; ++MB) {
      MachineBasicBlock::iterator MBB = MB->begin();
      const MachineBasicBlock::iterator MBE = MB->end();

      for (; MBB != MBE; ++MBB) {
        MachineInstr::mop_iterator MIB = MBB->operands_begin();
        const MachineInstr::mop_iterator MIE = MBB->operands_end();

        for (MachineInstr::mop_iterator MII = MIB; MII != MIE; ++MII) {
          if (!MII->isFI() || MII->getIndex() != FRI->first) continue;
          DEBUG(dbgs() << "FOUND FI#" << MII->getIndex() << "\n");
          MII->setIndex(NFI);
        }
      }
    }
  }
}

//===----------------------------------------------------------------------===//
//
// Stack Frame Processing methods
// +----------------------------+
//
// The stack is allocated decrementing the stack pointer on
// the first instruction of a function prologue. Once decremented,
// all stack references are are done through a positive offset
// from the stack/frame pointer, so the stack is considered
// to grow up.
//
//===----------------------------------------------------------------------===//

static void analyzeFrameIndexes(MachineFunction &MF) {
//FIXME: Not ported to Mico32
  if (Mico32DisableStackAdjust) return;

  MachineFrameInfo *MFI = MF.getFrameInfo();
  Mico32FunctionInfo *Mico32FI = MF.getInfo<Mico32FunctionInfo>();
  const MachineRegisterInfo &MRI = MF.getRegInfo();

  MachineRegisterInfo::livein_iterator LII = MRI.livein_begin();
  MachineRegisterInfo::livein_iterator LIE = MRI.livein_end();
  const SmallVector<int, 16> &LiveInFI = Mico32FI->getLiveIn();
  SmallVector<MachineInstr*, 16> EraseInstr;
  SmallVector<std::pair<int,int64_t>, 16> FrameRelocate;

  MachineBasicBlock *MBB = MF.getBlockNumbered(0);
  MachineBasicBlock::iterator MIB = MBB->begin();
  MachineBasicBlock::iterator MIE = MBB->end();

  int StackAdjust = 0;
  int StackOffset = -28;

  // In this loop we are searching frame indexes that corrospond to incoming
  // arguments that are already in the stack. We look for instruction sequences
  // like the following:
  //    
  //    LW REG, FI1, 0
  //    ...
  //    SW REG, FI2, 0
  //
  // As long as there are no defs of REG in the ... part, we can eliminate
  // the SW instruction because the value has already been stored to the
  // stack by the caller. All we need to do is locate FI at the correct
  // stack location according to the calling convensions.
  //
  // Additionally, if the SW operation kills the def of REG then we don't
  // need the LW operation so we can erase it as well.
  for (unsigned i = 0, e = LiveInFI.size(); i < e; ++i) {
    for (MachineBasicBlock::iterator I=MIB; I != MIE; ++I) {
      if (I->getOpcode() != Mico32::LW || I->getNumOperands() != 3 ||
          !I->getOperand(1).isFI() || !I->getOperand(0).isReg() ||
          I->getOperand(1).getIndex() != LiveInFI[i]) continue;

      unsigned FIReg = I->getOperand(0).getReg();
      MachineBasicBlock::iterator SI = I;
      for (SI++; SI != MIE; ++SI) {
        if (!SI->getOperand(0).isReg() ||
            !SI->getOperand(1).isFI() ||
            SI->getOpcode() != Mico32::SW) continue;

        int FI = SI->getOperand(1).getIndex();
        if (SI->getOperand(0).getReg() != FIReg ||
            MFI->isFixedObjectIndex(FI) ||
            MFI->getObjectSize(FI) != 4) continue;

        if (SI->getOperand(0).isDef()) break;

        if (SI->getOperand(0).isKill()) {
          DEBUG(dbgs() << "LW for FI#" << I->getOperand(1).getIndex() 
                       << " removed\n");
          EraseInstr.push_back(I);
        }

        EraseInstr.push_back(SI);
        DEBUG(dbgs() << "SW for FI#" << FI << " removed\n");

        FrameRelocate.push_back(std::make_pair(FI,StackOffset));
        DEBUG(dbgs() << "FI#" << FI << " relocated to " << StackOffset << "\n");

        StackOffset -= 4;
        StackAdjust += 4;
        break;
      }
    }
  }

  // In this loop we are searching for frame indexes that corrospond to
  // incoming arguments that are in registers. We look for instruction
  // sequences like the following:
  //    
  //    ...  SW REG, FI, 0
  // 
  // As long as the ... part does not define REG and if REG is an incoming
  // parameter register then we know that, according to ABI convensions, the
  // caller has allocated stack space for it already.  Instead of allocating
  // stack space on our frame, we record the correct location in the callers
  // frame.
  for (MachineRegisterInfo::livein_iterator LI = LII; LI != LIE; ++LI) {
    for (MachineBasicBlock::iterator I=MIB; I != MIE; ++I) {
      if (I->definesRegister(LI->first))
        break;

      if (I->getOpcode() != Mico32::SW || I->getNumOperands() != 3 ||
          !I->getOperand(1).isFI() || !I->getOperand(0).isReg() ||
          I->getOperand(1).getIndex() < 0) continue;

      if (I->getOperand(0).getReg() == LI->first) {
        int FI = I->getOperand(1).getIndex();
        Mico32FI->recordLiveIn(FI);

        int FILoc = 0;
        switch (LI->first) {
        default: llvm_unreachable("invalid incoming parameter!");
        case Mico32::R5:  FILoc = -4; break;
        case Mico32::R6:  FILoc = -8; break;
        case Mico32::R7:  FILoc = -12; break;
        case Mico32::R8:  FILoc = -16; break;
        case Mico32::R9:  FILoc = -20; break;
        case Mico32::R10: FILoc = -24; break;
        }

        StackAdjust += 4;
        FrameRelocate.push_back(std::make_pair(FI,FILoc));
        DEBUG(dbgs() << "FI#" << FI << " relocated to " << FILoc << "\n");
        break;
      }
    }
  }

  // Go ahead and erase all of the instructions that we determined were
  // no longer needed.
  for (int i = 0, e = EraseInstr.size(); i < e; ++i)
    MBB->erase(EraseInstr[i]);

  // Replace all of the frame indexes that we have relocated with new
  // fixed object frame indexes.
  replaceFrameIndexes(MF, FrameRelocate);
}

#if 0
static void interruptFrameLayout(MachineFunction &MF) {
//FIXME: Not ported to Mico32
  const Function *F = MF.getFunction();
  llvm::CallingConv::ID CallConv = F->getCallingConv();

  // If this function is not using either the interrupt_handler
  // calling convention or the save_volatiles calling convention
  // then we don't need to do any additional frame layout.
  if (CallConv != llvm::CallingConv::MICO32_INTR &&
      CallConv != llvm::CallingConv::MICO32_SVOL)
      return;

  MachineFrameInfo *MFI = MF.getFrameInfo();
  const MachineRegisterInfo &MRI = MF.getRegInfo();
  const Mico32InstrInfo &TII =
    *static_cast<const Mico32InstrInfo*>(MF.getTarget().getInstrInfo());

  // Determine if the calling convention is the interrupt_handler
  // calling convention. Some pieces of the prologue and epilogue
  // only need to be emitted if we are lowering and interrupt handler.
  //bool isIntr = CallConv == llvm::CallingConv::MICO32_INTR;
  bool isIntr =  true;

  // Determine where to put prologue and epilogue additions
  MachineBasicBlock &MENT   = MF.front();
  MachineBasicBlock &MEXT   = MF.back();

  MachineBasicBlock::iterator MENTI = MENT.begin();
  MachineBasicBlock::iterator MEXTI = prior(MEXT.end());

  DebugLoc ENTDL = MENTI != MENT.end() ? MENTI->getDebugLoc() : DebugLoc();
  DebugLoc EXTDL = MEXTI != MEXT.end() ? MEXTI->getDebugLoc() : DebugLoc();

  // Store the frame indexes generated during prologue additions for use
  // when we are generating the epilogue additions.
  SmallVector<int, 10> VFI;

  // Build the prologue SW for R3 - R12 if needed. Note that R11 must
  // always have a SW because it is used when processing RMSR.
  for (unsigned r = Mico32::R3; r <= Mico32::R12; ++r) {
    if (!MRI.isPhysRegUsed(r) && !(isIntr && r == Mico32::R11)) continue;
    
    int FI = MFI->CreateStackObject(4,4,false,false);
    VFI.push_back(FI);

    BuildMI(MENT, MENTI, ENTDL, TII.get(Mico32::SW), r)
      .addFrameIndex(FI).addImm(0);
  }
    
  // Build the prologue SW for R17, R18
  int R17FI = MFI->CreateStackObject(4,4,false,false);
  int R18FI = MFI->CreateStackObject(4,4,false,false);

  BuildMI(MENT, MENTI, ENTDL, TII.get(Mico32::SW), Mico32::R17)
    .addFrameIndex(R17FI).addImm(0);
    
  BuildMI(MENT, MENTI, ENTDL, TII.get(Mico32::SW), Mico32::R18)
    .addFrameIndex(R18FI).addImm(0);

  // Buid the prologue SW and the epilogue LW for RMSR if needed
  if (isIntr) {
    int MSRFI = MFI->CreateStackObject(4,4,false,false);
    BuildMI(MENT, MENTI, ENTDL, TII.get(Mico32::MFS), Mico32::R11)
      .addReg(Mico32::RMSR);
    BuildMI(MENT, MENTI, ENTDL, TII.get(Mico32::SW), Mico32::R11)
      .addFrameIndex(MSRFI).addImm(0);

    BuildMI(MEXT, MEXTI, EXTDL, TII.get(Mico32::LW), Mico32::R11)
      .addFrameIndex(MSRFI).addImm(0);
    BuildMI(MEXT, MEXTI, EXTDL, TII.get(Mico32::MTS), Mico32::RMSR)
      .addReg(Mico32::R11);
  }

  // Build the epilogue LW for R17, R18
  BuildMI(MEXT, MEXTI, EXTDL, TII.get(Mico32::LW), Mico32::R18)
    .addFrameIndex(R18FI).addImm(0);

  BuildMI(MEXT, MEXTI, EXTDL, TII.get(Mico32::LW), Mico32::R17)
    .addFrameIndex(R17FI).addImm(0);

  // Build the epilogue LW for R3 - R12 if needed
  for (unsigned r = Mico32::R12, i = VFI.size(); r >= Mico32::R3; --r) {
    if (!MRI.isPhysRegUsed(r)) continue;
    BuildMI(MEXT, MEXTI, EXTDL, TII.get(Mico32::LW), r)
      .addFrameIndex(VFI[--i]).addImm(0);
  }
}
#endif

static void determineFrameLayout(MachineFunction &MF) {
//FIXME: Not ported to Mico32
  MachineFrameInfo *MFI = MF.getFrameInfo();
  Mico32FunctionInfo *Mico32FI = MF.getInfo<Mico32FunctionInfo>();

  // Replace the dummy '0' SPOffset by the negative offsets, as explained on
  // LowerFORMAL_ARGUMENTS. Leaving '0' for while is necessary to avoid
  // the approach done by calculateFrameObjectOffsets to the stack frame.
  Mico32FI->adjustLoadArgsFI(MFI);
  Mico32FI->adjustStoreVarArgsFI(MFI);

  // Get the number of bytes to allocate from the FrameInfo
  unsigned FrameSize = MFI->getStackSize();
  DEBUG(dbgs() << "Original Frame Size: " << FrameSize << "\n" );

  // Get the alignments provided by the target, and the maximum alignment
  // (if any) of the fixed frame objects.
  // unsigned MaxAlign = MFI->getMaxAlignment();
  unsigned TargetAlign = MF.getTarget().getFrameLowering()->getStackAlignment();
  unsigned AlignMask = TargetAlign - 1;

  // Make sure the frame is aligned.
  FrameSize = (FrameSize + AlignMask) & ~AlignMask;
  MFI->setStackSize(FrameSize);
  DEBUG(dbgs() << "Aligned Frame Size: " << FrameSize << "\n" );
}

int Mico32FrameLowering::getFrameIndexOffset(const MachineFunction &MF, int FI) 
  const {
  const Mico32FunctionInfo *Mico32FI = MF.getInfo<Mico32FunctionInfo>();
  if (Mico32FI->hasReplacement(FI))
    FI = Mico32FI->getReplacement(FI);
  return TargetFrameLowering::getFrameIndexOffset(MF,FI);
}

// hasFP - Return true if the specified function should have a dedicated frame
// pointer register.  This is true if the function has variable sized allocas or
// if frame pointer elimination is disabled.
bool Mico32FrameLowering::hasFP(const MachineFunction &MF) const {
//FIXME: Not ported to Mico32
  const MachineFrameInfo *MFI = MF.getFrameInfo();
  return DisableFramePointerElim(MF) || MFI->hasVarSizedObjects();
}

void Mico32FrameLowering::emitPrologue(MachineFunction &MF) const {
//FIXME: Not ported to Mico32
  MachineBasicBlock &MBB   = MF.front();
  MachineFrameInfo *MFI    = MF.getFrameInfo();
  const Mico32InstrInfo &TII =
    *static_cast<const Mico32InstrInfo*>(MF.getTarget().getInstrInfo());
  Mico32FunctionInfo *Mico32FI = MF.getInfo<Mico32FunctionInfo>();
  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();

//  llvm::CallingConv::ID CallConv = MF.getFunction()->getCallingConv();
  //bool requiresRA = CallConv == llvm::CallingConv::MICO32_INTR;
  bool requiresRA =  true;

  // Determine the correct frame layout
  determineFrameLayout(MF);

  // Get the number of bytes to allocate from the FrameInfo.
  unsigned StackSize = MFI->getStackSize();

  // No need to allocate space on the stack.
  if (StackSize == 0 && !MFI->adjustsStack() && !requiresRA) return;

  int FPOffset = Mico32FI->getFPStackOffset();
  int RAOffset = Mico32FI->getRAStackOffset();

  // Adjust stack : addi R1, R1, -imm
  BuildMI(MBB, MBBI, DL, TII.get(Mico32::ADDI), Mico32::R1)
      .addReg(Mico32::R1).addImm(-StackSize);

  // swi  R15, R1, stack_loc
  if (MFI->adjustsStack() || requiresRA) {
    BuildMI(MBB, MBBI, DL, TII.get(Mico32::SW))
        .addReg(Mico32::R15).addReg(Mico32::R1).addImm(RAOffset);
  }

  if (hasFP(MF)) {
    // swi  R19, R1, stack_loc
    BuildMI(MBB, MBBI, DL, TII.get(Mico32::SW))
      .addReg(Mico32::R19).addReg(Mico32::R1).addImm(FPOffset);

    // add R19, R1, R0
    BuildMI(MBB, MBBI, DL, TII.get(Mico32::ADD), Mico32::R19)
      .addReg(Mico32::R1).addReg(Mico32::R0);
  }
}

void Mico32FrameLowering::emitEpilogue(MachineFunction &MF,
                                   MachineBasicBlock &MBB) const {
//FIXME: Not ported to Mico32
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  MachineFrameInfo *MFI            = MF.getFrameInfo();
  Mico32FunctionInfo *Mico32FI     = MF.getInfo<Mico32FunctionInfo>();
  const Mico32InstrInfo &TII =
    *static_cast<const Mico32InstrInfo*>(MF.getTarget().getInstrInfo());

  DebugLoc dl = MBBI->getDebugLoc();

//  llvm::CallingConv::ID CallConv = MF.getFunction()->getCallingConv();
  //bool requiresRA = CallConv == llvm::CallingConv::MICO32_INTR;
  bool requiresRA =  true;

  // Get the FI's where RA and FP are saved.
  int FPOffset = Mico32FI->getFPStackOffset();
  int RAOffset = Mico32FI->getRAStackOffset();

  if (hasFP(MF)) {
    // add R1, R19, R0
    BuildMI(MBB, MBBI, dl, TII.get(Mico32::ADD), Mico32::R1)
      .addReg(Mico32::R19).addReg(Mico32::R0);

    // lwi  R19, R1, stack_loc
    BuildMI(MBB, MBBI, dl, TII.get(Mico32::LW), Mico32::R19)
      .addReg(Mico32::R1).addImm(FPOffset);
  }

  // lwi R15, R1, stack_loc
  if (MFI->adjustsStack() || requiresRA) {
    BuildMI(MBB, MBBI, dl, TII.get(Mico32::LW), Mico32::R15)
      .addReg(Mico32::R1).addImm(RAOffset);
  }

  // Get the number of bytes from FrameInfo
  int StackSize = (int) MFI->getStackSize();

  // addi R1, R1, imm
  if (StackSize) {
    BuildMI(MBB, MBBI, dl, TII.get(Mico32::ADDI), Mico32::R1)
      .addReg(Mico32::R1).addImm(StackSize);
  }
}

void Mico32FrameLowering::
processFunctionBeforeCalleeSavedScan(MachineFunction &MF,
                                     RegScavenger *RS) const {
//FIXME: Not ported to Mico32
  MachineFrameInfo *MFI = MF.getFrameInfo();
  Mico32FunctionInfo *Mico32FI = MF.getInfo<Mico32FunctionInfo>();
//  llvm::CallingConv::ID CallConv = MF.getFunction()->getCallingConv();
  //bool requiresRA = CallConv == llvm::CallingConv::MICO32_INTR;
  bool requiresRA = true;

  if (MFI->adjustsStack() || requiresRA) {
    Mico32FI->setRAStackOffset(0);
    MFI->CreateFixedObject(4,0,true);
  }

  if (hasFP(MF)) {
    Mico32FI->setFPStackOffset(4);
    MFI->CreateFixedObject(4,4,true);
  }

  //interruptFrameLayout(MF);
  analyzeFrameIndexes(MF);
}
