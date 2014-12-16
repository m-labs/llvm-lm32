//===-- LM32FrameLowering.h - Frame info for LM32 Target ---------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains LM32 frame information that doesn't fit anywhere else
// cleanly...
//
// Based on XCore.
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LM32_LM32FRAMELOWERING_H
#define LLVM_LIB_TARGET_LM32_LM32FRAMELOWERING_H

#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
  class LM32Subtarget;

  class LM32FrameLowering: public TargetFrameLowering {
    const LM32Subtarget &Subtarget;
  public:
    LM32FrameLowering(const LM32Subtarget &subtarget);

    /// Determine the frame's layout
    void determineFrameLayout(MachineFunction &MF) const;

    /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
    /// the function.
    void emitPrologue(MachineFunction &MF) const override;
    void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

    // Is this frame using a Frame Pointer?
    bool hasFP(const MachineFunction &MF) const override;
    
    /// hasReservedCallFrame - Under normal circumstances, when a frame pointer is
    /// not required, we reserve argument space for call sites in the function
    /// immediately on entry to the current function. This eliminates the need for
    /// add/sub sp brackets around call sites. Returns true if the call frame is
    /// included as part of the stack frame.
    /// LM32 always reserves call frames. eliminateCallFramePseudoInstr() may
    /// need to change if this does.  See ARM for example.
    ///
    /// PEI::calculateFrameObjectOffsets() adds maxCallFrameSize if 
    /// hasReservedCallFrame() is true.  The stack alignment is set
    /// in LM32TargetMachine::LM32TargetMachine().
    /// Some targets (e.g. SPU, PPC) add in the maxCallFrameSize to the
    /// stacksize manually due to special alignment requirements or other issues.
    bool hasReservedCallFrame(const MachineFunction &MF) const override {
      // Why wouldn't you do this:
      // It's not always a good idea to include the call frame as part of the
      // stack frame. ARM (especially Thumb) has small immediate offset to
      // address the stack frame. So a large call frame can cause poor codegen
      // and may even makes it impossible to scavenge a register.
      //
      return true;
    }

    void eliminateCallFramePseudoInstr(MachineFunction &MF,
                                       MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator MI) const;

    /// processFunctionBeforeFrameFinalized - This method is called immediately
    /// before the specified functions frame layout (MF.getFrameInfo()) is
    /// finalized.  Once the frame is finalized, MO_FrameIndex operands are
    /// replaced with direct constants.  This method is optional.
    void processFunctionBeforeFrameFinalized(MachineFunction &MF,
                                             RegScavenger *RS = nullptr) const override;

    /// processFunctionBeforeCalleeSavedScan - This method is called immediately
    /// before PrologEpilogInserter scans the physical registers used to 
    /// determine what callee saved registers should be spilled. This method 
    /// is optional.
    void processFunctionBeforeCalleeSavedScan(MachineFunction &MF,
                                              RegScavenger *RS = nullptr) const override;
    
    //! Stack slot size (4 bytes)
    static int stackSlotSize() {
      return 4;
    }
  };
}

#endif // LLVM_LIB_TARGET_LM32_LM32FRAMELOWERING_H
