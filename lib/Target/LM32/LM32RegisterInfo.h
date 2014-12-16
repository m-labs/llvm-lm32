//===- LM32RegisterInfo.h - LM32 Register Information Impl --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the LM32 implementation of the TargetRegisterInfo
// class.
//
//===----------------------------------------------------------------------===//

#ifndef LM32REGISTERINFO_H
#define LM32REGISTERINFO_H

#include "llvm/Target/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "LM32GenRegisterInfo.inc"


namespace llvm {
class LM32Subtarget;
class TargetInstrInfo;

struct LM32RegisterInfo : public LM32GenRegisterInfo {
  LM32Subtarget &Subtarget;

  LM32RegisterInfo(LM32Subtarget &st);

  /// Code Generation virtual methods...
  const uint16_t *getCalleeSavedRegs(const MachineFunction* MF = 0) const;

  BitVector getReservedRegs(const MachineFunction &MF) const;

  /// Stack Frame Processing Methods
  /// eliminateFrameIndex - This method must be overriden to eliminate abstract
  /// frame indices from instructions which may use them.  The instruction
  /// referenced by the iterator contains an MO_FrameIndex operand which must be
  /// eliminated by this method.  This method may modify or replace the
  /// specified instruction, as long as it keeps the iterator pointing the the
  /// finished product. SPAdj is the SP adjustment due to call frame setup
  /// instruction.
  void eliminateFrameIndex(MachineBasicBlock::iterator II,
                           int SPAdj, unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;


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
