//===- LM32RegisterInfo.cpp - LM32 Register Information -== -*- C++ -*-===//
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

#define DEBUG_TYPE "lm32-frame-info"

#include "LM32.h"
#include "LM32Subtarget.h"
#include "LM32RegisterInfo.h"
#include "LM32MachineFunctionInfo.h"
#include "llvm/Constants.h"
#include "llvm/Type.h"
#include "llvm/Function.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"

#define GET_REGINFO_TARGET_DESC
#include "LM32GenRegisterInfo.inc"

using namespace llvm;

LM32RegisterInfo::
LM32RegisterInfo(const LM32Subtarget &ST, const TargetInstrInfo &tii)
  : LM32GenRegisterInfo(LM32::RRA), Subtarget(ST), TII(tii) {}


//===----------------------------------------------------------------------===//
// Callee Saved Registers methods
//===----------------------------------------------------------------------===//

/// LM32 Callee Saved Registers
/// getCalleeSavedRegs - Return a null-terminated list of all of the
/// callee saved registers on this target. The register should be in the
/// order of desired callee-save stack frame offset. The first register is
/// close to the incoming stack pointer if stack grows down, and vice versa.
/// Copied from GCC lm32.h
const unsigned* LM32RegisterInfo::
getCalleeSavedRegs(const MachineFunction *MF) const {
  // LM32 callee-save register range is R11 - R28
  static const unsigned CalleeSavedRegs[] = {
    LM32::R11, LM32::R12, LM32::R13, LM32::R14,
    LM32::R15, LM32::R16, LM32::R17, LM32::R18,
    LM32::R19, LM32::R20, LM32::R21, LM32::R22,
    LM32::R23, LM32::R24, LM32::R25, 
    // R26 - R31 are reserved.
    0
  };

  return CalleeSavedRegs;
}

/// getReservedRegs - Returns a bitset indexed by physical register number
/// indicating if a register is a special register that has particular uses
/// and should be considered unavailable at all times, e.g. SP (R28). This is
/// used by register scavenger to determine what registers are free.
/// Copied from GCC lm32.h
BitVector LM32RegisterInfo::
getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  Reserved.set(LM32::R0);  // Always 0
  Reserved.set(LM32::RGP); // R26 - GP register
  Reserved.set(LM32::RFP); // R27 - Frame Pointer (GCC not fixed)
  Reserved.set(LM32::RSP); // R28 - Stack Pointer
  Reserved.set(LM32::RRA); // R29 - Return Address (GCC not fixed)
  Reserved.set(LM32::REA); // R30 - Exception Address
  Reserved.set(LM32::RBA); // R31 - Breakpoint Address
  return Reserved;
}

// This function eliminate ADJCALLSTACKDOWN/ADJCALLSTACKUP pseudo instructions
void LM32RegisterInfo::
eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator I) const {
  // We're assuming a fixed call frame. hasReservedCallFrame==true
  // Since we are using reserved call frames and we don't need to adjust
  // the stack pointer outside of the prologue/epilogue we'll just erase
  // the ADJCALLSTACKDOWN, ADJCALLSTACKUP instructions.
  // Normally these would bracket calls and be used for dynamic call frames.
  // Note alloca() is handled in the prologue.
  MBB.erase(I);
}

/// eliminateFrameIndex - This method must be overriden to eliminate abstract
/// frame indices from instructions which may use them.  The instruction
/// referenced by the iterator contains an MO_FrameIndex operand which must be
/// eliminated by this method.  This method may modify or replace the
/// specified instruction, as long as it keeps the iterator pointing the the
/// finished product. SPAdj is the SP adjustment due to call frame setup
/// instruction. The call frame setup instructions is defined in the 
/// TargetRegisterInfo constructor (e.g. ADJCALLSTACKUP/ADJCALLSTACKDOWN).
/// The call frame setup instructions are processed by
/// eliminateCallFramePseudoInstr()
///
//  FIXME: So what is SPAdj?
/// This is where stack pointer or frame pointer encoding is done.
///
/// Typically the FrameIndex operands are added by storeRegToStackSlot() and
/// loadRegFromStackSlot() using addFrameIndex() and are added to target 
/// specific instructions.
///
/// There appears to be a convention of adding an immediate value to the
/// frame index amongst most of the targets, hence the 
/// MI.getOperand(i+1).getImm() added to the frame index. For most targets, 
/// and specifically Monarch, this immediate value is zero.
void LM32RegisterInfo::
eliminateFrameIndex(MachineBasicBlock::iterator II,
                    int SPAdj, RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected");

  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo *MFrmInf = MF.getFrameInfo();
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();
  bool useFP = TFI->hasFP(MF);

  // SP points to the top of stack (possibly biased to first free location), 
  // FP points to the callee save registers at top of the frame (the last 
  // used stack location prior to entering the function).
  unsigned FPRegToUse = LM32::RSP;
  if (useFP) {
    FPRegToUse = LM32::RFP;
  }

  // Find the frame index operand.
  unsigned i = 0;
  while (!MI.getOperand(i).isFI()) {
    ++i;
    assert(i < MI.getNumOperands() && "Instr doesn't have FrameIndex operand!");
  }
  MachineOperand &FrameOp = MI.getOperand(i);
  int FrameIndex = FrameOp.getIndex();

  int Offset = MF.getFrameInfo()->getObjectOffset(FrameIndex);
  assert(Offset%4 == 0 && "Object has misaligned stack offset");

  int StackSize = MF.getFrameInfo()->getStackSize();
//  LM32FunctionInfo *MFuncInf = MF.getInfo<LM32FunctionInfo>();
  
#ifndef NDEBUG
  DEBUG(dbgs() << "\nFunction         : "
               << MF.getFunction()->getName() << "\n");
  DEBUG(dbgs() << "<--------->\n");
  DEBUG(MI.print(dbgs()));
  DEBUG(dbgs() << "FrameIndex         : " << FrameIndex << "\n");
  DEBUG(dbgs() << "FrameOffset        : " << Offset << "\n");
  DEBUG(dbgs() << "StackSize          : " << StackSize << "\n");
  DEBUG(dbgs() << "AdjustedOffset     : " << Offset + StackSize << "\n");
  DEBUG(dbgs() << "FPreg              : "
               << ((LM32::RFP == FPRegToUse) ? "FP" : "SP") << "\n");
  DEBUG(dbgs() << "getImm()           : "
               <<  MI.getOperand(i + 1).getImm() << "\n");
  DEBUG(dbgs() << "isFixed            : "
               << MFrmInf->isFixedObjectIndex(FrameIndex) << "\n");
// DEBUG(dbgs() << "isLiveIn           : "
//               << MFuncInf->isLiveIn(FrameIndex) << "\n");
  DEBUG(dbgs() << "isSpill            : "
               << MFrmInf->isSpillSlotObjectIndex(FrameIndex) << "\n");
#endif

  if (!useFP) {
    // We're using the SP - it points to the bottom of the frame, offset to top.
    Offset += StackSize;
    assert(Offset%4 == 0 && "Misaligned stack offset");
    Offset += Subtarget.hasSPBias()? 4 : 0;
    DEBUG(dbgs() << "Offset from biased SP: " << Offset << "\n");
  }

  // Fold constant into offset.
  // I believe this can happen with vector code - it needs to be checked.
//  assert(MI.getOperand(i + 1).getImm() == 0 && "need to check this code path");
  Offset += MI.getOperand(i + 1).getImm();
  MI.getOperand(i + 1).ChangeToImmediate(0);

  // This shows up in vector code:
  //  assert(Offset%4 == 0 && "Misaligned stack offset");

#ifndef NDEBUG
  DEBUG(dbgs() << "Offset from FP/SP : " << Offset << "\n");
#endif

  // Replace frame index with a stack/frame pointer reference.
  if (Offset >= -32768 && Offset <= 32767) {
    // If the offset is small enough to fit in the immediate field, directly
    // encode it.
    // Replace the FrameIndex with the appropriate frame pointer 
    // register RSP (28) or RFP (27).
    FrameOp.ChangeToRegister(FPRegToUse, false);
    MI.getOperand(i+1).ChangeToImmediate(Offset);
  } else {
    assert( 0 && "Unimplemented - frame index limited to 32767 byte offset.");
  }
#ifndef NDEBUG
  DEBUG(errs() << "<--RESULT->\n");
  DEBUG(MI.print(errs()));
  DEBUG(errs() << "<--------->\n");
#endif

  return;
}


//===----------------------------------------------------------------------===//
// Stack Frame Processing methods
//===----------------------------------------------------------------------===//

// copied from X86.  Arm supports stack realignment too.
// Note RealignStack is set by realign-stack option, true by default.
bool LM32RegisterInfo::
canRealignStack(const MachineFunction &MF) const {
  const MachineFrameInfo *MFI = MF.getFrameInfo();
  return (MF.getTarget().Options.RealignStack &&
          !MFI->hasVarSizedObjects());
}

// copied from X86.  Arm supports stack realignment too.
bool LM32RegisterInfo::
needsStackRealignment(const MachineFunction &MF) const {
  const MachineFrameInfo *MFI = MF.getFrameInfo();
  const Function *F = MF.getFunction();
  unsigned StackAlign = MF.getTarget().getFrameLowering()->getStackAlignment();
  DEBUG(if (!(MFI->getMaxAlignment() <= StackAlign))
             errs() << "\nError in function: "
                    << MF.getFunction()->getName() << "\n");
  assert((MFI->getMaxAlignment() <= StackAlign)  &&
            "Unable to align stack to requested alignment.");
  assert(((MFI->getMaxAlignment() <= StackAlign) || 
            MF.getTarget().Options.RealignStack) && 
            "Unable to align stack to requested alignment.");
  bool requiresRealignment = ((MFI->getMaxAlignment() > StackAlign) ||
                               F->hasFnAttr(Attribute::StackAlignment));

  // FIXME: Currently we don't support stack realignment for functions with
  //        variable-sized allocas.
  // FIXME: It's more complicated than this...
  if (0 && requiresRealignment && MFI->hasVarSizedObjects())
    report_fatal_error(
      "Stack realignment in presence of dynamic allocas is not supported");

  // If we've requested that we force align the stack do so now.
  // this is x86 specific.  Note that hasFP changes too if this is supported.
//  if (ForceStackAlign)
//    return canRealignStack(MF);

  return requiresRealignment && canRealignStack(MF);
}


unsigned LM32RegisterInfo::getRARegister() const {
  return LM32::RRA;
}

unsigned LM32RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();

  return TFI->hasFP(MF) ? LM32::RFP : LM32::RSP;
}

unsigned LM32RegisterInfo::getEHExceptionRegister() const {
  llvm_unreachable("What is the exception register");
  return 0;
}

unsigned LM32RegisterInfo::getEHHandlerRegister() const {
  llvm_unreachable("What is the exception handler register");
  return 0;
}
