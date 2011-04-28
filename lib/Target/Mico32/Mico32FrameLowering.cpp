//=====- Mico32FrameLowering.cpp - Mico32 Frame Information ------ C++ -*-====//
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

#include "Mico32.h"
#include "Mico32FrameLowering.h"
#include "Mico32InstrInfo.h"
#include "Mico32MachineFunctionInfo.h"
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

using namespace llvm;

Mico32FrameLowering::Mico32FrameLowering(const Mico32Subtarget &sti)
  : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, 4, 0),
    STI(sti) {
  // Do nothing
}


/// getInitialFrameState - Returns a list of machine moves that are assumed
/// on entry to a function.  From XCore.
void Mico32FrameLowering::
getInitialFrameState(std::vector<MachineMove> &Moves) const {
  // Initial state of the frame pointer is RSP (SP).
  MachineLocation Dst(MachineLocation::VirtualFP);
  MachineLocation Src(Mico32::RSP, 0);
  Moves.push_back(MachineMove(0, Dst, Src));
}


/// determineFrameLayout - Align the frame and maximum call frame and
/// updated the sizes. Copied from SPU.
void Mico32FrameLowering::
determineFrameLayout(MachineFunction &MF) const {
  unsigned AlignMask = 32 - 1;  
  MachineFrameInfo *MFI = MF.getFrameInfo();

  // Get the maximum call frame size of all the calls.
  unsigned maxCallFrameSize = MFI->getMaxCallFrameSize();

  // If we have maxCallFrameSize needs to be 32 byte aligned so
  // that allocations will be aligned.
  maxCallFrameSize = (maxCallFrameSize + AlignMask) & ~AlignMask;
  
  // Update maximum call frame size.
  MFI->setMaxCallFrameSize(maxCallFrameSize);
  
  // Get the number of bytes to allocate from the FrameInfo
  unsigned FrameSize = MFI->getStackSize();
  //assert((FrameSize % 32) == 0 && "The stack should be 32 byte aligned.");

  // Make sure the stack is 32 byte aligned.
  FrameSize = (FrameSize + AlignMask) & ~AlignMask;

  // Include call frame size in total.
  FrameSize += maxCallFrameSize;
  
  // Update frame info.
  MFI->setStackSize(FrameSize);
}


/// hasFP - Return true if the specified function should have a dedicated frame
/// pointer register.  This is true if the stacksize > 0 and either the 
/// function has variable sized allocas or frame pointer elimination 
/// is disabled.
///
bool Mico32FrameLowering::
hasFP(const MachineFunction &MF) const {
  return DisableFramePointerElim(MF) || MF.getFrameInfo()->hasVarSizedObjects();
}


// Generate the function prologue.
// This is based on XCore with some SPU influence.
void Mico32FrameLowering::
emitPrologue(MachineFunction &MF) const {
  MachineBasicBlock &MBB = MF.front();    // Prolog goes in entry BB
  MachineBasicBlock::iterator MBBI = MBB.begin();
  MachineFrameInfo *MFrmInf = MF.getFrameInfo();
  MachineModuleInfo *MMI = &MF.getMMI();

  Mico32FunctionInfo *MonFI = MF.getInfo<Mico32FunctionInfo>();
  const Mico32InstrInfo &TII =
    *static_cast<const Mico32InstrInfo*>(MF.getTarget().getInstrInfo());

  DebugLoc dl = (MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc());

  // determineFrameLayout() sets some stack related sizes so do it first.
  determineFrameLayout(MF);

  // Get the maximum call frame size of all the calls.
  unsigned maxCallFrameSize = MFrmInf->getMaxCallFrameSize();
  assert(maxCallFrameSize%32 == 0 && "Misaligned call frame size");

  bool FP = hasFP(MF);

  // Get the frame size.
  int FrameSize = MFrmInf->getStackSize();
  assert(FrameSize%32 == 0 && 
         "Mico32FrameLowering::emitPrologue Misaligned frame size");
  
  bool emitFrameMoves = MMI->hasDebugInfo() ||
                          !MF.getFunction()->doesNotThrow() || 
                          UnwindTablesMandatory;

  assert(((MonFI->getUsesLR() && FrameSize) || !MonFI->getUsesLR()) && 
         "we should have a frame if LR is used.");
  assert(((FP && FrameSize) || !FP) && "we should have a frame if FP is used.");

  // Do we need to allocate space on the stack?
  if (FrameSize) {
    // adjust the stack pointer: RSP -= numbytes
    // Note we can only add 32767 offset in emitEpilogue()
    // so we don't use 32768. See SPU for model of how to implement larger
    // stack frames.
    if (FrameSize < 32768) {
      BuildMI(MBB, MBBI, dl, TII.get(Mico32::ADDI),
              Mico32::RSP).addReg(Mico32::RSP).addImm(-FrameSize);
    } else {
      // We could use multiple instructions to generate the offset.
      report_fatal_error("Unhandled frame size: " + Twine(FrameSize));
    }

    // Add the "machine moves" for the instructions we generated above, but in
    // reverse order.
    // Mapping for machine moves (see MCDwarf.cpp):
    //
    //   DST: VirtualFP 
    //        SRC: VirtualFP              => DW_CFA_def_cfa_offset
    //        SRC: others                 => DW_CFA_def_cfa
    //
    //   ELSE  (DST != VirtualFP)
    //   SRC: VirtualFP
    //        DST: Register               => DW_CFA_def_cfa_register
    //        DST: other                  => Error
    //
    //   ELSE 
    //        OFFSET < 0                  => DW_CFA_offset_extended_sf
    //        REG < 64                    => DW_CFA_offset + Reg
    //        ELSE                        => DW_CFA_offset_extended
    //
    if (emitFrameMoves) {
      std::vector<MachineMove> &Moves = MMI->getFrameMoves();

      // Show update of SP.
      MCSymbol *FrameLabel = MMI->getContext().CreateTempSymbol();
      BuildMI(MBB, MBBI, dl, TII.get(Mico32::PROLOG_LABEL)).addSym(FrameLabel);

      // Generate DW_CFA_def_cfa_offset for SP.
      MachineLocation SPDst(MachineLocation::VirtualFP);
      MachineLocation SPSrc(MachineLocation::VirtualFP, FrameSize);
      Moves.push_back(MachineMove(FrameLabel, SPDst, SPSrc));
    }
    
    if (MonFI->getUsesLR()) {
      // Save the LR (RRA) to the preallocated stack slot.
      int LRSpillOffset = MFrmInf->getObjectOffset(MonFI->getLRSpillSlot())
        +FrameSize;
      BuildMI(MBB, MBBI, dl, TII.get(Mico32::SW))
        .addReg(Mico32::RSP).addImm(LRSpillOffset).addReg(Mico32::RRA);
      MBB.addLiveIn(Mico32::RRA);
      
      if (emitFrameMoves) {
        MCSymbol *SaveLRLabel = MMI->getContext().CreateTempSymbol();
        BuildMI(MBB, MBBI, dl, TII.get(Mico32::PROLOG_LABEL))
                .addSym(SaveLRLabel);
        MachineLocation CSDst(MachineLocation::VirtualFP, LRSpillOffset);
        MachineLocation CSSrc(Mico32::RRA);
        MMI->getFrameMoves().push_back(MachineMove(SaveLRLabel, CSDst, CSSrc));
      }
    }
  } 
  
  if (FP) {
    // Save FP (R27) to the stack.
    int FPSpillOffset = 
      MFrmInf->getObjectOffset(MonFI->getFPSpillSlot())+FrameSize;

    BuildMI(MBB, MBBI, dl, TII.get(Mico32::SW))
      .addReg(Mico32::RSP).addImm(FPSpillOffset).addReg(Mico32::RFP);

    // RFP is live-in. It is killed at the spill.
    MBB.addLiveIn(Mico32::RFP);
    if (emitFrameMoves) {
      MCSymbol *SaveRFPLabel = MMI->getContext().CreateTempSymbol();
      BuildMI(MBB, MBBI, dl, TII.get(Mico32::PROLOG_LABEL))
              .addSym(SaveRFPLabel);
      MachineLocation CSDst(MachineLocation::VirtualFP, FPSpillOffset);
      MachineLocation CSSrc(Mico32::RFP);
      MMI->getFrameMoves().push_back(MachineMove(SaveRFPLabel, CSDst, CSSrc));
    }
    // Set the FP from the SP.
    unsigned FramePtr = Mico32::RFP;
    // The FP points to the beginning of the preallocated outgoing arguments, 
    // hence the maxCallFrameSize.
    BuildMI(MBB, MBBI, dl, TII.get(Mico32::ADDI),FramePtr)
      .addReg(Mico32::RSP).addImm(maxCallFrameSize);
    if (emitFrameMoves) {
      // Show FP is now valid.
      MCSymbol *FrameLabel = MMI->getContext().CreateTempSymbol();
      BuildMI(MBB, MBBI, dl, TII.get(Mico32::PROLOG_LABEL)).addSym(FrameLabel);
      MachineLocation SPDst(FramePtr);
      MachineLocation SPSrc(MachineLocation::VirtualFP);
      MMI->getFrameMoves().push_back(MachineMove(FrameLabel, SPDst, SPSrc));
    }
  }
  
#if 0
  if (emitFrameMoves) {
    // Frame moves for callee saved.
    std::vector<MachineMove> &Moves = MMI->getFrameMoves();
    std::vector<std::pair<MCSymbol*, CalleeSavedInfo> >&SpillLabels =
        MonFI->getSpillLabels();
    for (unsigned I = 0, E = SpillLabels.size(); I != E; ++I) {
      MCSymbol *SpillLabel = SpillLabels[I].first;
      CalleeSavedInfo &CSI = SpillLabels[I].second;
      int Offset = MFI->getObjectOffset(CSI.getFrameIdx());
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
    BuildMI(MBB, MBBI, dl, TII.get(Mico32::PROLOG_LABEL)).addSym(FrameLabel);
    MCSymbol *ReadyLabel = 0;

    MCSymbol *Label = FP ? ReadyLabel : FrameLabel;

    // Add callee saved registers to move list.
    const std::vector<CalleeSavedInfo> &CSI = MFrmInf->getCalleeSavedInfo();
    for (unsigned I = 0, E = CSI.size(); I != E; ++I) {
      int Offset = MFrmInf->getObjectOffset(CSI[I].getFrameIdx());
      unsigned Reg = CSI[I].getReg();
      //if (Reg == PPC::LR || Reg == PPC::LR8 || Reg == PPC::RM) continue;
      MachineLocation CSDst(MachineLocation::VirtualFP, Offset);
      MachineLocation CSSrc(Reg);
      MMI->getFrameMoves().push_back(MachineMove(Label, CSDst, CSSrc));
    }
  }

}


void Mico32FrameLowering::
emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const {
  MachineFrameInfo *MFI = MF.getFrameInfo();
  MachineBasicBlock::iterator MBBI = prior(MBB.end());
  const Mico32InstrInfo &TII =
    *static_cast<const Mico32InstrInfo*>(MF.getTarget().getInstrInfo());
  Mico32FunctionInfo *MonFI = MF.getInfo<Mico32FunctionInfo>();
  assert(MBBI->getOpcode() == Mico32::RET &&
         "Can only put epilog before 'ret' instruction!");
  DebugLoc dl = MBBI->getDebugLoc();
  unsigned FramePtr = Mico32::RFP;
  
  bool FP = hasFP(MF);

  // Get the number of bytes to allocate from the FrameInfo
  int NumBytes = MFI->getStackSize();

  if (MonFI->getUsesLR()) {
    // Restore the LR (RRA) from the preallocated stack slot.
    int LRSpillOffset = MFI->getObjectOffset(MonFI->getLRSpillSlot())
      +NumBytes;
    BuildMI(MBB, MBBI, dl, TII.get(Mico32::LW))
      .addReg(Mico32::RRA).addReg(Mico32::RSP).addImm(LRSpillOffset);
  }
      
  // if using allocas
  // FP -> SP,   Note the FP is biased by MFI->getMaxCallFrameSize()
  // restore FP
  // SP +=  MFI->getStackSize() - MFI->getMaxCallFrameSize();
  if (FP) {
    // FP -> SP.
    BuildMI(MBB, MBBI, dl, TII.get(Mico32::ADDI),Mico32::RSP)
      .addReg(FramePtr).addImm(0);
    // account for the FP bias from the SP
    NumBytes -= MFI->getMaxCallFrameSize();
    assert(NumBytes != 0 && "Should always have a frame if there is an FP use.");
  }

  if (NumBytes) {

    if (FP) {
      // Restore FP from the stack, note SP has been biased by
      // getMaxCallFrameSize() earlier.
      int FPSpillOffset = MFI->getObjectOffset(MonFI->getFPSpillSlot()) + NumBytes;

      BuildMI(MBB, MBBI, dl, TII.get(Mico32::LW)).addReg(Mico32::RFP)
        .addReg(Mico32::RSP).addImm(FPSpillOffset);
    }

    // SP +=  MFI->getStackSize() - MFI->getMaxCallFrameSize();
    if (NumBytes < 32768) {
      BuildMI(MBB, MBBI, dl, TII.get(Mico32::ADDI), Mico32::RSP)
        .addReg(Mico32::RSP).addImm(NumBytes);
    } else {
      assert( 0 && "Unimplemented - per function stack size limited to 32767 bytes.");
    }
  }
}


/// processFunctionBeforeFrameFinalized - This method is called immediately
/// before the specified functions frame layout (MF.getFrameInfo()) is
/// finalized.  Once the frame is finalized, MO_FrameIndex operands are
/// replaced with direct constants.  This method is optional.
void Mico32FrameLowering::
processFunctionBeforeFrameFinalized(MachineFunction &MF) const {}

/// processFunctionBeforeCalleeSavedScan - This method is called immediately
/// before PrologEpilogInserter scans the physical registers used to determine
/// what callee saved registers should be spilled. This method is optional.
/// ARM provides a complex example of this function.
/// The following is based on XCore
void Mico32FrameLowering::
processFunctionBeforeCalleeSavedScan(MachineFunction &MF,
                                     RegScavenger *RS) const {
  MachineFrameInfo *MFI = MF.getFrameInfo();
  const TargetRegisterInfo *RegInfo = MF.getTarget().getRegisterInfo();
  bool LRUsed = MF.getRegInfo().isPhysRegUsed(Mico32::RRA);
  const TargetRegisterClass *RC = Mico32::GPRRegisterClass;
  Mico32FunctionInfo *MonFI = MF.getInfo<Mico32FunctionInfo>();

//   MFI->dump(MF);

// FIXME:  I think the ret always marks RA used - in which case this is wrong.
  assert(LRUsed && "See if LR is always used.  If this is a leaf I assume it will not be used.");
	
  if (LRUsed) {
    MF.getRegInfo().setPhysRegUnused(Mico32::RRA);
    
    bool isVarArg = MF.getFunction()->isVarArg();
    int FrameIdx;
    if (! isVarArg) {
      // CreateFixedObject allocates space relative to the SP on entry to
      // the function.  Since SP(0) is occupied by incoming arguments (or
      // whatever) we need to allocate the next slot on the stack (e.g. -4).
      FrameIdx = MFI->CreateFixedObject(RC->getSize(), -4, true);
    } else {
      assert(0 && "Test Vararg frames.");
      FrameIdx = MFI->CreateStackObject(RC->getSize(),
                                        RC->getAlignment(),
                                        false);
    }
    MonFI->setUsesLR(FrameIdx);
    MonFI->setLRSpillSlot(FrameIdx);
  }
  if (RegInfo->requiresRegisterScavenging(MF)) {
    assert(0 && "Test register scavenging.");
    // Reserve a slot close to SP or frame pointer.
    RS->setScavengingFrameIndex(MFI->CreateStackObject(RC->getSize(),
                                                       RC->getAlignment(),
                                                       false));
  }
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();
  if (TFI->hasFP(MF)) {
    // FP is a callee save register.
    // This needs saving / restoring in the epilogue / prologue.
    // Supposedly FP is marked live-in and is killed at the spill. So
    // don't bother marking it unused.
    //MF.getRegInfo().setPhysRegUnused(Mico32::RFP);
    // FIXME: shouldn't isSS be true?  XCore says no...
    MonFI->setFPSpillSlot(MFI->CreateFixedObject(RC->getSize(),(LRUsed)?-8:-4,
                                                 true));
  }
}

