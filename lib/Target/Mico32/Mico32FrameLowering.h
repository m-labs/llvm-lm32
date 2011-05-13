//===-- Mico32FrameLowering.h - Frame info for Mico32 Target -----*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains Mico32 frame information that doesn't fit anywhere else
// cleanly...
//
// Based on XCore.
//===----------------------------------------------------------------------===//

#ifndef MICO32FRAMEINFO_H
#define MICO32FRAMEINFO_H

#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
  class Mico32Subtarget;

  class Mico32FrameLowering: public TargetFrameLowering {
    const Mico32Subtarget &Subtarget;
  public:
    Mico32FrameLowering(const Mico32Subtarget &subtarget);

    /// Determine the frame's layout
    void determineFrameLayout(MachineFunction &MF) const;

    /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
    /// the function.
    void emitPrologue(MachineFunction &MF) const;
    void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const;

    // Is this frame using a Frame Pointer?
    bool hasFP(const MachineFunction &MF) const;

    /// getInitialFrameState - Returns a list of machine moves that are assumed
    /// on entry to a function.
    void getInitialFrameState(std::vector<MachineMove> &Moves) const;

    /// processFunctionBeforeFrameFinalized - This method is called immediately
    /// before the specified functions frame layout (MF.getFrameInfo()) is
    /// finalized.  Once the frame is finalized, MO_FrameIndex operands are
    /// replaced with direct constants.  This method is optional.
    void processFunctionBeforeFrameFinalized(MachineFunction &MF) const;

    /// processFunctionBeforeCalleeSavedScan - This method is called immediately
    /// before PrologEpilogInserter scans the physical registers used to 
    /// determine what callee saved registers should be spilled. This method 
    /// is optional.
    void processFunctionBeforeCalleeSavedScan(MachineFunction &MF,
                                                RegScavenger *RS = NULL) const;
    
    //! Stack slot size (4 bytes)
    static int stackSlotSize() {
      return 4;
    }
  };
}

#endif // MICO32FRAMEINFO_H
