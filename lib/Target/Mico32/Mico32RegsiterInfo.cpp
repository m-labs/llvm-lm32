//===- Mico32RegisterInfo.cpp - Mico32 Register Information -== -*- C++ -*-===//
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

#define DEBUG_TYPE "mico32-frame-info"

#include "Mico32.h"
#include "Mico32Subtarget.h"
#include "Mico32RegisterInfo.h"
#include "Mico32MachineFunction.h"
#include "llvm/Constants.h"
#include "llvm/Type.h"
#include "llvm/Function.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineLocation.h"
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

using namespace llvm;

Mico32RegisterInfo::
Mico32RegisterInfo(const Mico32Subtarget &ST, const TargetInstrInfo &tii)
  : Mico32GenRegisterInfo(Mico32::ADJCALLSTACKDOWN, Mico32::ADJCALLSTACKUP),
    Subtarget(ST), TII(tii) {}


//===----------------------------------------------------------------------===//
// Callee Saved Registers methods
//===----------------------------------------------------------------------===//

/// Mico32 Callee Saved Registers
/// getCalleeSavedRegs - Return a null-terminated list of all of the
/// callee saved registers on this target. The register should be in the
/// order of desired callee-save stack frame offset. The first register is
/// close to the incoming stack pointer if stack grows down, and vice versa.
/// Copied from GCC lm32.h
const unsigned* Mico32RegisterInfo::
getCalleeSavedRegs(const MachineFunction *MF) const {
  // Mico32 callee-save register range is R11 - R28
  static const unsigned CalleeSavedRegs[] = {
    Mico32::R11, Mico32::R12, Mico32::R13, Mico32::R14,
    Mico32::R15, Mico32::R16, Mico32::R17, Mico32::R18,
    Mico32::R19, Mico32::R20, Mico32::R21, Mico32::R22,
    Mico32::R23, Mico32::R24, Mico32::R25, 
    Mico32::R28, Mico32::R29, 
    0
  };

  return CalleeSavedRegs;
}

/// getReservedRegs - Returns a bitset indexed by physical register number
/// indicating if a register is a special register that has particular uses
/// and should be considered unavailable at all times, e.g. SP (R28). This is
/// used by register scavenger to determine what registers are free.
/// Copied from GCC lm32.h
BitVector Mico32RegisterInfo::
getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  Reserved.set(Mico32::R0);  // Always 0
  Reserved.set(Mico32::R26); // GP register
//  Reserved.set(Mico32::R27); // Frame Pointer (GCC not fixed)
  Reserved.set(Mico32::R28); // Stack Pointer
//  Reserved.set(Mico32::R29); // Return Address (GCC not fixed)
  Reserved.set(Mico32::R30); // Exception Address
  Reserved.set(Mico32::R31); // Breakpoint Address
  return Reserved;
}

// This function eliminate ADJCALLSTACKDOWN/ADJCALLSTACKUP pseudo instructions
void Mico32RegisterInfo::
eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator I) const {
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();

  if (!TFI->hasReservedCallFrame(MF)) {
    // If we have a frame pointer, turn the adjcallstackup instruction into a
    // 'addi r1, r1, -<amt>' and the adjcallstackdown instruction into
    // 'addi r1, r1, <amt>'
    MachineInstr *Old = I;
    int Amount = Old->getOperand(0).getImm() + 4;
    if (Amount != 0) {
      // We need to keep the stack aligned properly.  To do this, we round the
      // amount of space needed for the outgoing arguments up to the next
      // alignment boundary.
      unsigned Align = TFI->getStackAlignment();
      Amount = (Amount+Align-1)/Align*Align;

      MachineInstr *New;
      if (Old->getOpcode() == Mico32::ADJCALLSTACKDOWN) {
        New = BuildMI(MF,Old->getDebugLoc(),TII.get(Mico32::ADDI),Mico32::R28)
                .addReg(Mico32::R28).addImm(-Amount);
      } else {
        assert(Old->getOpcode() == Mico32::ADJCALLSTACKUP);
        New = BuildMI(MF,Old->getDebugLoc(),TII.get(Mico32::ADDI),Mico32::R28)
                .addReg(Mico32::R28).addImm(Amount);
      }

      // Replace the pseudo instruction with a new instruction...
      MBB.insert(I, New);
    }
  }

  // Simply discard ADJCALLSTACKDOWN, ADJCALLSTACKUP instructions.
  MBB.erase(I);
}

/// eliminateFrameIndex - This method must be overriden to eliminate abstract
/// frame indices from instructions which may use them.  The instruction
/// referenced by the iterator contains an MO_FrameIndex operand which must be
/// eliminated by this method.  This method may modify or replace the
/// specified instruction, as long as it keeps the iterator pointing the the
/// finished product. SPAdj is the SP adjustment due to call frame setup
/// instruction.
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

// From MBlaze:
// FrameIndex represent objects inside a abstract stack.
// We must replace FrameIndex with an stack/frame pointer
// direct reference.
void Mico32RegisterInfo::
eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                    RegScavenger *RS) const {
  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  MachineFrameInfo *MFI = MF.getFrameInfo();

  unsigned i = 0;
  while (!MI.getOperand(i).isFI()) {
    ++i;
    assert(i < MI.getNumOperands() &&
           "Instr doesn't have FrameIndex operand!");
  }

  unsigned oi = i == 2 ? 1 : 2;

  DEBUG(dbgs() << "\nFunction : " << MF.getFunction()->getName() << "\n";
        dbgs() << "<--------->\n" << MI);

  int FrameIndex = MI.getOperand(i).getIndex();
  int stackSize  = MFI->getStackSize();
  int spOffset   = MFI->getObjectOffset(FrameIndex);

  DEBUG(Mico32FunctionInfo *Mico32FI = MF.getInfo<Mico32FunctionInfo>();
        dbgs() << "FrameIndex : " << FrameIndex << "\n"
               << "spOffset   : " << spOffset << "\n"
               << "stackSize  : " << stackSize << "\n"
               << "isFixed    : " << MFI->isFixedObjectIndex(FrameIndex) << "\n"
               << "isLiveIn   : " << Mico32FI->isLiveIn(FrameIndex) << "\n"
               << "isSpill    : " << MFI->isSpillSlotObjectIndex(FrameIndex)
               << "\n" );

  // as explained on LowerFormalArguments, detect negative offsets
  // and adjust SPOffsets considering the final stack size.
  int Offset = (spOffset < 0) ? (stackSize - spOffset) : spOffset;
  Offset += MI.getOperand(oi).getImm();

  DEBUG(dbgs() << "Offset     : " << Offset << "\n" << "<--------->\n");

  MI.getOperand(oi).ChangeToImmediate(Offset);
  MI.getOperand(i).ChangeToRegister(getFrameRegister(MF), false);
}

/// From MBlaze:
/// processFunctionBeforeFrameFinalized - This method is called immediately
/// before the specified functions frame layout (MF.getFrameInfo()) is
/// finalized.  Once the frame is finalized, MO_FrameIndex operands are
/// replaced with direct constants.  This method is optional.
void Mico32RegisterInfo::
processFunctionBeforeFrameFinalized(MachineFunction &MF) const {
  // Set the stack offset where GP must be saved/loaded from.
  MachineFrameInfo *MFI = MF.getFrameInfo();
  Mico32FunctionInfo *Mico32FI = MF.getInfo<Mico32FunctionInfo>();
  if (Mico32FI->needGPSaveRestore())
    MFI->setObjectOffset(Mico32FI->getGPFI(), Mico32FI->getGPStackOffset());
}


unsigned Mico32RegisterInfo::getRARegister() const {
  return Mico32::R29;
}

unsigned Mico32RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();

  return TFI->hasFP(MF) ? Mico32::R27 : Mico32::R28;
}

unsigned Mico32RegisterInfo::getEHExceptionRegister() const {
  llvm_unreachable("What is the exception register");
  return 0;
}

unsigned Mico32RegisterInfo::getEHHandlerRegister() const {
  llvm_unreachable("What is the exception handler register");
  return 0;
}

int Mico32RegisterInfo::getDwarfRegNum(unsigned RegNo, bool isEH) const {
  return Mico32GenRegisterInfo::getDwarfRegNumFull(RegNo,0);
}

#include "Mico32GenRegisterInfo.inc"

