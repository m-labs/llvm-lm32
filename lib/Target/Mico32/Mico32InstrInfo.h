//===- Mico32InstrInfo.h - Mico32 Instruction Information -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Mico32 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef MICO32INSTRUCTIONINFO_H
#define MICO32INSTRUCTIONINFO_H

#include "Mico32.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "Mico32RegisterInfo.h"

#define GET_INSTRINFO_HEADER
#include "Mico32GenInstrInfo.inc"

namespace llvm {

namespace Mico32 {

  inline static bool isUncondBranchOpcode(int Opc) {
    switch (Opc) {
    default: return false;
    case Mico32::BI:
    case Mico32::B:
      return true;
    }
  }

  inline static bool isCondBranchOpcode(int Opc) {
    switch (Opc) {
    default: return false;
    case Mico32::BE:
    case Mico32::BG:
    case Mico32::BGE:
    case Mico32::BGEU:
    case Mico32::BGU:
    case Mico32::BNE:
      return true;
    }
  }
}


class Mico32InstrInfo : public Mico32GenInstrInfo {
  Mico32TargetMachine &TM;
  const Mico32RegisterInfo RI;
public:
  explicit Mico32InstrInfo(Mico32TargetMachine &TM);

  /// getRegisterInfo - TargetInstrInfo is a superset of MRegister info.  As
  /// such, whenever a client has an instance of instruction info, it should
  /// always be able to get register info as well (through this method).
  ///
  virtual const Mico32RegisterInfo &getRegisterInfo() const { return RI; }

#if 0
  /// isLoadFromStackSlot - If the specified machine instruction is a direct
  /// load from a stack slot, return the virtual or physical register number of
  /// the destination along with the FrameIndex of the loaded stack slot.  If
  /// not, return 0.  This predicate must return 0 if the instruction has
  /// any side effects other than loading from the stack slot.
  virtual unsigned isLoadFromStackSlot(const MachineInstr *MI,
                                       int &FrameIndex) const;

  /// isStoreToStackSlot - If the specified machine instruction is a direct
  /// store to a stack slot, return the virtual or physical register number of
  /// the source reg along with the FrameIndex of the loaded stack slot.  If
  /// not, return 0.  This predicate must return 0 if the instruction has
  /// any side effects other than storing to the stack slot.
  virtual unsigned isStoreToStackSlot(const MachineInstr *MI,
                                      int &FrameIndex) const;
#endif

  /// Branch Analysis
  virtual bool AnalyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                             MachineBasicBlock *&FBB,
                             SmallVectorImpl<MachineOperand> &Cond,
                             bool AllowModify) const;
  virtual unsigned InsertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                                MachineBasicBlock *FBB,
                                const SmallVectorImpl<MachineOperand> &Cond,
                                DebugLoc DL) const;
  virtual unsigned RemoveBranch(MachineBasicBlock &MBB) const;

  virtual bool ReverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond)
    const;


  virtual void copyPhysReg(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator I, DebugLoc DL,
                           unsigned DestReg, unsigned SrcReg,
                           bool KillSrc) const;
  virtual void storeRegToStackSlot(MachineBasicBlock &MBB,
                                   MachineBasicBlock::iterator MBBI,
                                   unsigned SrcReg, bool isKill, int FrameIndex,
                                   const TargetRegisterClass *RC,
                                   const TargetRegisterInfo *TRI) const;

  virtual void loadRegFromStackSlot(MachineBasicBlock &MBB,
                                    MachineBasicBlock::iterator MBBI,
                                    unsigned DestReg, int FrameIndex,
                                    const TargetRegisterClass *RC,
                                    const TargetRegisterInfo *TRI) const;
#if 0 

  /// Insert nop instruction when hazard condition is found
  virtual void insertNoop(MachineBasicBlock &MBB,
                          MachineBasicBlock::iterator MI) const;

#endif

#if 0
  /// getGlobalBaseReg - Return a virtual register initialized with the
  /// the global base register value. Output instructions required to
  /// initialize the register in the function entry block, if necessary.
  /// getGlobalBaseReg - insert code into the entry mbb to materialize the PIC
  /// base register.  Return the virtual register that holds this value.
  ///
  unsigned getGlobalBaseReg(MachineFunction *MF) const;
#endif

  static MachineBasicBlock *GetBranchTarget(const MachineInstr& inst);
};

}
#endif
