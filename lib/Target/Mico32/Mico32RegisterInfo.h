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

  // Handle cases where the stack must be aligned more than the default
  // alignment.
  bool needsStackRealignment(const MachineFunction &MF) const;
  bool canRealignStack(const MachineFunction &MF) const;


  /// Debug information queries.
  unsigned getRARegister() const;
  unsigned getFrameRegister(const MachineFunction &MF) const;

  /// Exception handling queries.
  unsigned getEHExceptionRegister() const;
  unsigned getEHHandlerRegister() const;
};

} // end namespace llvm

#endif
