//===- Mico32RegisterInfo.h - Mico32 Register Information Impl --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Mico32 implementation of the TargetRegisterInfo
// class.
//
//===----------------------------------------------------------------------===//

#ifndef MICO32REGISTERINFO_H
#define MICO32REGISTERINFO_H

#include "Mico32.h"
#include "llvm/Target/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "Mico32GenRegisterInfo.inc"


namespace llvm {
class Mico32Subtarget;
class TargetInstrInfo;
class Type;

struct Mico32RegisterInfo : public Mico32GenRegisterInfo {
  const Mico32Subtarget &Subtarget;
  const TargetInstrInfo &TII;

  Mico32RegisterInfo(const Mico32Subtarget &Subtarget,
                     const TargetInstrInfo &tii);

  /// Code Generation virtual methods...
  const unsigned *getCalleeSavedRegs(const MachineFunction* MF = 0) const;

  BitVector getReservedRegs(const MachineFunction &MF) const;


  /// hasReservedCallFrame - Under normal circumstances, when a frame pointer is
  /// not required, we reserve argument space for call sites in the function
  /// immediately on entry to the current function. This eliminates the need for
  /// add/sub sp brackets around call sites. Returns true if the call frame is
  /// included as part of the stack frame.
  /// Mico32 always reserves call frames. eliminateCallFramePseudoInstr() may
  /// need to change if this does.  See ARM for example.
  ///
  /// PEI::calculateFrameObjectOffsets() adds maxCallFrameSize if 
  /// hasReservedCallFrame() is true.  The stack alignment is set
  /// in Mico32TargetMachine::Mico32TargetMachine().
  /// Some targets (e.g. SPU, PPC) add in the maxCallFrameSize to the
  /// stacksize manually due to special alignment requirements or other issues.
  bool hasReservedCallFrame(const MachineFunction &MF) const {
    // Why wouldn't you do this:
    // It's not always a good idea to include the call frame as part of the
    // stack frame. ARM (especially Thumb) has small immediate offset to
    // address the stack frame. So a large call frame can cause poor codegen
    // and may even makes it impossible to scavenge a register.
    //
    return true;
  }

  /// eliminateCallFramePseudoInstr - This method is called during prolog/epilog
  /// code insertion to eliminate call frame setup and destroy pseudo
  /// instructions (but only if the Target is using them).  It is responsible
  /// for eliminating these instructions, replacing them with concrete
  /// instructions.  This method need only be implemented if using call frame
  /// setup/destroy pseudo instructions.
  void eliminateCallFramePseudoInstr(MachineFunction &MF,
                                     MachineBasicBlock &MBB,
                                     MachineBasicBlock::iterator I) const;

  /// Stack Frame Processing Methods
  /// eliminateFrameIndex - This method must be overriden to eliminate abstract
  /// frame indices from instructions which may use them.  The instruction
  /// referenced by the iterator contains an MO_FrameIndex operand which must be
  /// eliminated by this method.  This method may modify or replace the
  /// specified instruction, as long as it keeps the iterator pointing the the
  /// finished product. SPAdj is the SP adjustment due to call frame setup
  /// instruction.
  void eliminateFrameIndex(MachineBasicBlock::iterator II,
                           int SPAdj, RegScavenger *RS = NULL) const;

  /// processFunctionBeforeFrameFinalized - This method is called immediately
  /// before the specified functions frame layout (MF.getFrameInfo()) is
  /// finalized.  Once the frame is finalized, MO_FrameIndex operands are
  /// replaced with direct constants.  This method is optional.
  void processFunctionBeforeFrameFinalized(MachineFunction &MF) const;

  /// Debug information queries.
  unsigned getRARegister() const;
  unsigned getFrameRegister(const MachineFunction &MF) const;

  /// Exception handling queries.
  unsigned getEHExceptionRegister() const;
  unsigned getEHHandlerRegister() const;
};

} // end namespace llvm

#endif
