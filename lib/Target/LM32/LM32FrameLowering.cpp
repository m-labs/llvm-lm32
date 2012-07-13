//=====- LM32FrameLowering.cpp - LM32 Frame Information ---------- C++ -*-====//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the LM32 implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "LM32.h"
#include "LM32FrameLowering.h"
#include "LM32InstrInfo.h"
#include "LM32MachineFunctionInfo.h"
#include "llvm/Function.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "LM32Subtarget.h"


using namespace llvm;

LM32FrameLowering::LM32FrameLowering(const LM32Subtarget &subtarget)
  : TargetFrameLowering(TargetFrameLowering::StackGrowsDown,
                        4 /*StackAlignment*/, 
                        0 /*LocalAreaOffset*/,
                        4 /*TransientStackAlignment*/),
      Subtarget(subtarget) {
  // Do nothing
}


/// hasFP - Return true if the specified function should have a dedicated frame
/// pointer register.  This is true if the stacksize > 0 and either the 
/// function has variable sized allocas or frame pointer elimination 
/// is disabled or stack alignment is less than requested alignment.
///
bool LM32FrameLowering::
hasFP(const MachineFunction &MF) const {
  const TargetRegisterInfo *RegInfo = MF.getTarget().getRegisterInfo();
  const MachineFrameInfo *MFI = MF.getFrameInfo();

  return (MF.getTarget().Options.DisableFramePointerElim(MF) || 
           RegInfo->needsStackRealignment(MF) ||
           MF.getFrameInfo()->hasVarSizedObjects() ||
           MFI->isFrameAddressTaken());
}


// Generate the function prologue.
// This is based on XCore with some SPU influence.
void LM32FrameLowering::
emitPrologue(MachineFunction &MF) const {
  MachineBasicBlock &MBB = MF.front();    // Prolog goes in entry BB
  MachineBasicBlock::iterator MBBI = MBB.begin();
  MachineFrameInfo *MFrmInf = MF.getFrameInfo();
  MachineModuleInfo *MMI = &MF.getMMI();

  LM32FunctionInfo *MFuncInf = MF.getInfo<LM32FunctionInfo>();
  const LM32InstrInfo &TII =
    *static_cast<const LM32InstrInfo*>(MF.getTarget().getInstrInfo());

  DebugLoc dl = (MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc());

  // Check the maximum call frame size of all the calls is sensible.
  assert(MFrmInf->getMaxCallFrameSize()%4 == 0 && "Misaligned call frame size");

  bool FP = hasFP(MF);
  bool needsRealignment =
           MF.getTarget().getRegisterInfo()->needsStackRealignment(MF);


  // Get the frame size.
  int FrameSize = MFrmInf->getStackSize();
  assert(FrameSize%4 == 0 && 
         "LM32FrameLowering::emitPrologue Misaligned frame size");
  
  
  bool emitFrameMoves = MMI->hasDebugInfo() ||
                        MF.getFunction()->needsUnwindTableEntry();

  assert(((MFuncInf->getUsesLR() && FrameSize) || !MFuncInf->getUsesLR()) && 
         "we should have a frame if LR is used.");
  assert(((FP && FrameSize) || !FP) && "we should have a frame if FP is used.");

  // Do we need to allocate space on the stack?
  if (FrameSize) {
    // adjust the stack pointer: RSP -= FrameSize
    // Note we can only add 32767 offset in emitEpilogue()
    // so we don't use 32768. See SPU for model of how to implement larger
    // stack frames.
    if (FrameSize < 32768) {
      BuildMI(MBB, MBBI, dl, TII.get(LM32::ADDI),
              LM32::RSP).addReg(LM32::RSP).addImm(-FrameSize);
    } else {
      // We could use multiple instructions to generate the offset.
      report_fatal_error("Unhandled frame size in function: " + MBB.getParent()->getFunction()->getName() + " size: " + Twine(FrameSize));
    }

    // Add the "machine moves" for the instructions we generated above, but in
    // reverse order.
    // Mapping for machine moves (see MCDwarf.cpp):
    if (emitFrameMoves) {
      std::vector<MachineMove> &Moves = MMI->getFrameMoves();

      // Show update of SP.
      MCSymbol *FrameLabel = MMI->getContext().CreateTempSymbol();
      BuildMI(MBB, MBBI, dl, TII.get(LM32::PROLOG_LABEL)).addSym(FrameLabel);

  DEBUG(dbgs() << "\nFunction: "
               << MF.getFunction()->getName() << " SP Frame debug location\n");
  DEBUG(dbgs() << "Moving VirtFP: \n");
  DEBUG(dbgs() << "VirtualFP = VirtFP[" << FrameSize << "]\n");
      // Generate DW_CFA_def_cfa_offset for SP.
      MachineLocation SPDst(MachineLocation::VirtualFP);
      MachineLocation SPSrc(MachineLocation::VirtualFP, FrameSize);
      Moves.push_back(MachineMove(FrameLabel, SPDst, SPSrc));
    }
    
    if (MFuncInf->getUsesLR()) {
      // Save the LR (RRA) to the preallocated stack slot.
      int LRSpillOffset = MFrmInf->getObjectOffset(MFuncInf->getLRSpillSlot())
        +FrameSize;
      LRSpillOffset += Subtarget.hasSPBias()? 4 : 0;
      BuildMI(MBB, MBBI, dl, TII.get(LM32::SW))
        .addReg(LM32::RRA).addReg(LM32::RSP).addImm(LRSpillOffset);
      MBB.addLiveIn(LM32::RRA);
      
      if (emitFrameMoves) {
        MCSymbol *SaveLRLabel = MMI->getContext().CreateTempSymbol();
        BuildMI(MBB, MBBI, dl, TII.get(LM32::PROLOG_LABEL))
                .addSym(SaveLRLabel);
  DEBUG(dbgs() << "\nFunction: "
               << MF.getFunction()->getName() << " LR Frame debug location\n");
  DEBUG(dbgs() << "Moving %RRA (link register): \n");
  DEBUG(dbgs() << "VirtualFP[FPSpillOffset] = VirtFP[" << LRSpillOffset << "]\n");
        MachineLocation CSDst(MachineLocation::VirtualFP, LRSpillOffset);
        MachineLocation CSSrc(LM32::RRA);
        MMI->getFrameMoves().push_back(MachineMove(SaveLRLabel, CSDst, CSSrc));
      }
    }
  
    if (FP) {
      // Save FP (R27) to the stack.
      int FPSpillOffset = 
        MFrmInf->getObjectOffset(MFuncInf->getFPSpillSlot())+FrameSize;
      FPSpillOffset += Subtarget.hasSPBias()? 4 : 0;
  
      BuildMI(MBB, MBBI, dl, TII.get(LM32::SW))
        .addReg(LM32::RFP).addReg(LM32::RSP).addImm(FPSpillOffset);
  
      // RFP is live-in. It is killed at the spill.
      MBB.addLiveIn(LM32::RFP);
      if (emitFrameMoves) {
             DEBUG(dbgs() << "\nFunction: "
               << MF.getFunction()->getName() << " FP Frame debug location\n");
  DEBUG(dbgs() << "Moving FPreg:              : "
               << ((FP) ? "FP" : "SP") << " to \n");
  DEBUG(dbgs() << "VirtualFP[FPSpillOffset] = VirtFP[" << FPSpillOffset << "]\n");
        MCSymbol *SaveRFPLabel = MMI->getContext().CreateTempSymbol();
        BuildMI(MBB, MBBI, dl, TII.get(LM32::PROLOG_LABEL))
                .addSym(SaveRFPLabel);
        MachineLocation CSDst(MachineLocation::VirtualFP, FPSpillOffset);
        MachineLocation CSSrc(LM32::RFP);
        MMI->getFrameMoves().push_back(MachineMove(SaveRFPLabel, CSDst, CSSrc));
      }
      // Set the FP from the SP.
      unsigned FramePtr = LM32::RFP;
      // The FP points to the beginning of the frame ( = SP on entry), 
      // hence we add in the FrameSize.
      FrameSize += Subtarget.hasSPBias()? 4 : 0;
      BuildMI(MBB, MBBI, dl, TII.get(LM32::ADDI),FramePtr)
        .addReg(LM32::RSP).addImm(FrameSize);
      if (emitFrameMoves) {
        DEBUG(dbgs() << "FRAMEMOVE: FPreg: is now FP \n");
        // Show FP is now valid.
        MCSymbol *FrameLabel = MMI->getContext().CreateTempSymbol();
        BuildMI(MBB, MBBI, dl,TII.get(LM32::PROLOG_LABEL)).addSym(FrameLabel);
        MachineLocation SPDst(FramePtr);
        MachineLocation SPSrc(MachineLocation::VirtualFP);
        MMI->getFrameMoves().push_back(MachineMove(FrameLabel, SPDst, SPSrc));
      }
    }
  } 
  
#if 0
  // from XCore (I believe):
  if (emitFrameMoves) {
    // Frame moves for callee saved.
    std::vector<MachineMove> &Moves = MMI->getFrameMoves();
    std::vector<std::pair<MCSymbol*, CalleeSavedInfo> >&SpillLabels =
        MFuncInf->getSpillLabels();
    for (unsigned I = 0, E = SpillLabels.size(); I != E; ++I) {
      MCSymbol *SpillLabel = SpillLabels[I].first;
      CalleeSavedInfo &CSI = SpillLabels[I].second;
      int Offset = MFrmInf->getObjectOffset(CSI.getFrameIdx());
      unsigned Reg = CSI.getReg();
      MachineLocation CSDst(MachineLocation::VirtualFP, Offset);
      MachineLocation CSSrc(Reg);
      Moves.push_back(MachineMove(SpillLabel, CSDst, CSSrc));
    }
  }
#endif
  // This is from PPC:
  if (emitFrameMoves) {
    MCSymbol *FrameLabel = MMI->getContext().CreateTempSymbol();
    BuildMI(MBB, MBBI, dl, TII.get(LM32::PROLOG_LABEL)).addSym(FrameLabel);
    MCSymbol *ReadyLabel = 0;

    MCSymbol *Label = FP ? ReadyLabel : FrameLabel;
    int bias = 0;
    if (!FP && Subtarget.hasSPBias())
      bias = 4;

    // Add callee saved registers to move list.
    const std::vector<CalleeSavedInfo> &CSI = MFrmInf->getCalleeSavedInfo();
    for (unsigned I = 0, E = CSI.size(); I != E; ++I) {
      int FrameIndex = CSI[I].getFrameIdx(); 
      int Offset = MFrmInf->getObjectOffset(CSI[I].getFrameIdx());
      unsigned Reg = CSI[I].getReg();
     DEBUG(dbgs() << "\nFunction         : "
               << MF.getFunction()->getName() << " FRAME MOVES\n");
  DEBUG(dbgs() << "<--------->\n");
  DEBUG(dbgs() << "FPreg              : "
               << ((FP) ? "FP" : "SP") << "\n");
  DEBUG(dbgs() << "Moving reg         : " << Reg << "\n");
  DEBUG(dbgs() << "FrameIndex         : " << FrameIndex << "\n");
  DEBUG(dbgs() << "FrameOffset        : " << Offset << "\n");
  DEBUG(dbgs() << "bias               : " << bias << "\n");
      Offset += bias;
      //if (Reg == PPC::LR || Reg == PPC::LR8 || Reg == PPC::RM) continue;
      MachineLocation CSDst(MachineLocation::VirtualFP, Offset);
      MachineLocation CSSrc(Reg);
      MMI->getFrameMoves().push_back(MachineMove(Label, CSDst, CSSrc));
    }
  }

}


void LM32FrameLowering::
emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const {
  MachineFrameInfo *MFrmInf = MF.getFrameInfo();
  MachineBasicBlock::iterator MBBI = prior(MBB.end());
  const LM32InstrInfo &TII =
    *static_cast<const LM32InstrInfo*>(MF.getTarget().getInstrInfo());
  LM32FunctionInfo *MFuncInf = MF.getInfo<LM32FunctionInfo>();
  assert(MBBI->getOpcode() == LM32::RET &&
         "Can only put epilog before 'ret' instruction!");
  DebugLoc dl = MBBI->getDebugLoc();
  
  bool FP = hasFP(MF);

  // Get the number of bytes allocated from the FrameInfo
  int FrameSize = MFrmInf->getStackSize();

  if (FrameSize) {
    if (FP) {
      // Restore FP (R27) from the stack.
      int FPSpillOffset = 
        MFrmInf->getObjectOffset(MFuncInf->getFPSpillSlot())+FrameSize;
      FPSpillOffset += Subtarget.hasSPBias()? 4 : 0;
      BuildMI(MBB, MBBI, dl, TII.get(LM32::LW))
        .addReg(LM32::RFP).addReg(LM32::RSP).addImm(FPSpillOffset);
    }
  
    if (MFuncInf->getUsesLR()) {
      // Restore the LR (RRA) from the preallocated stack slot.
      int LRSpillOffset = MFrmInf->getObjectOffset(MFuncInf->getLRSpillSlot())
          +FrameSize;
      LRSpillOffset += Subtarget.hasSPBias()? 4 : 0;
      BuildMI(MBB, MBBI, dl, TII.get(LM32::LW))
        .addReg(LM32::RRA).addReg(LM32::RSP).addImm(LRSpillOffset);
    }

    // SP +=  MFrmInf->getStackSize()
    if (FrameSize < 32768) {
      BuildMI(MBB, MBBI, dl, TII.get(LM32::ADDI), LM32::RSP)
        .addReg(LM32::RSP).addImm(FrameSize);
    } else {
      assert( 0 && "Unimplemented - per function stack size limited to 32767 bytes.");
    }
  }
}


/// processFunctionBeforeFrameFinalized - This method is called immediately
/// before the specified functions frame layout (MF.getFrameInfo()) is
/// finalized.  Once the frame is finalized, MO_FrameIndex operands are
/// replaced with direct constants.  This method is optional.
void LM32FrameLowering::
processFunctionBeforeFrameFinalized(MachineFunction &MF) const {}

/// processFunctionBeforeCalleeSavedScan - This method is called immediately
/// before PrologEpilogInserter scans the physical registers used to determine
/// what callee saved registers should be spilled. This method is optional.
/// ARM provides a complex example of this function.
/// The following is based on XCore
void LM32FrameLowering::
processFunctionBeforeCalleeSavedScan(MachineFunction &MF,
                                     RegScavenger *RS) const {
  MachineFrameInfo *MFrmInf = MF.getFrameInfo();
  const TargetRegisterInfo *RegInfo = MF.getTarget().getRegisterInfo();
  const TargetRegisterClass *RC = LM32::GPRRegisterClass;
  LM32FunctionInfo *MFuncInf = MF.getInfo<LM32FunctionInfo>();
  const bool LRUsed = MF.getRegInfo().isPhysRegUsed(LM32::RRA);
  const bool hasFP = MF.getTarget().getFrameLowering()->hasFP(MF);
  const bool isVarArg = MF.getFunction()->isVarArg();
  
  // CreateFixedObject allocates space relative to the SP on entry to
  // the function.  Since SP(0) is occupied by incoming arguments (or
  // whatever) we need to allocate the next slot on the stack (e.g. -4).
  int offset = -4;

  // For variadic functions we copy the parameters passed in registers
  // onto the stack in the first positions in the frame.  Therefore
  // we don't fix the location of the LR and FP here, The ABI doesn't
  // specify where they should be anyways.

  // LR can be optimized out prior to now:
  int FrameIdx;
  if (LRUsed) {
    MF.getRegInfo().setPhysRegUnused(LM32::RRA);
    
    if ( !isVarArg )
      FrameIdx = MFrmInf->CreateFixedObject(RC->getSize(), offset, true);
    else
      //FIXME: I'm lazy and didn't bother to assign the actual location.
      FrameIdx = MFrmInf->CreateStackObject(RC->getSize(),
	                                    RC->getAlignment(), false);
    offset -= 4;
    MFuncInf->setUsesLR(FrameIdx);
    MFuncInf->setLRSpillSlot(FrameIdx);
  }

  if (RegInfo->requiresRegisterScavenging(MF)) {
    assert(0 && "Test register scavenging.");
    // Reserve a slot close to SP or frame pointer.
    RS->setScavengingFrameIndex(MFrmInf->CreateStackObject(RC->getSize(),
                                                       RC->getAlignment(),
                                                       false));
  }

  if (hasFP) {
    // FP is a callee save register.
    // This needs saving / restoring in the epilogue / prologue.
    // Supposedly FP is marked live-in and is killed at the spill. So
    // don't bother marking it unused.
    //MF.getRegInfo().setPhysRegUnused(LM32::RFP);
    // FIXME: shouldn't isSS be true?  XCore says no...
    if ( !isVarArg )
      FrameIdx = MFrmInf->CreateFixedObject(RC->getSize(), offset, true);
    else
      //FIXME: I'm lazy and didn't bother to assign the actual location.
      FrameIdx = MFrmInf->CreateStackObject(RC->getSize(),
	                                    RC->getAlignment(), false);
    MFuncInf->setFPSpillSlot(FrameIdx);
  }
}
