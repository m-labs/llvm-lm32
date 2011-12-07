//=- llvm/CodeGen/DFAPacketizer.cpp - DFA Packetizer for VLIW -*- C++ -*-=====//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This class implements a deterministic finite automaton (DFA) based
// packetizing mechanism for VLIW architectures. It provides APIs to
// determine whether there exists a legal mapping of instructions to
// functional unit assignments in a packet. The DFA is auto-generated from
// the target's Schedule.td file.
//
// A DFA consists of 3 major elements: states, inputs, and transitions. For
// the packetizing mechanism, the input is the set of instruction classes for
// a target. The state models all possible combinations of functional unit
// consumption for a given set of instructions in a packet. A transition
// models the addition of an instruction to a packet. In the DFA constructed
// by this class, if an instruction can be added to a packet, then a valid
// transition exists from the corresponding state. Invalid transitions
// indicate that the instruction cannot be added to the current packet.
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/DFAPacketizer.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCInstrItineraries.h"
using namespace llvm;

DFAPacketizer::DFAPacketizer(const InstrItineraryData *I, const int (*SIT)[2],
                             const unsigned *SET):
  InstrItins(I), CurrentState(0), DFAStateInputTable(SIT),
  DFAStateEntryTable(SET) {}


//
// ReadTable - Read the DFA transition table and update CachedTable.
//
// Format of the transition tables:
// DFAStateInputTable[][2] = pairs of <Input, Transition> for all valid
//                           transitions
// DFAStateEntryTable[i] = Index of the first entry in DFAStateInputTable
//                         for the ith state
//
void DFAPacketizer::ReadTable(unsigned int state) {
  unsigned ThisState = DFAStateEntryTable[state];
  unsigned NextStateInTable = DFAStateEntryTable[state+1];
  // Early exit in case CachedTable has already contains this
  // state's transitions.
  if (CachedTable.count(UnsignPair(state,
                                   DFAStateInputTable[ThisState][0])))
    return;

  for (unsigned i = ThisState; i < NextStateInTable; i++)
    CachedTable[UnsignPair(state, DFAStateInputTable[i][0])] =
      DFAStateInputTable[i][1];
}


// canReserveResources - Check if the resources occupied by a MCInstrDesc
// are available in the current state.
bool DFAPacketizer::canReserveResources(const llvm::MCInstrDesc *MID) {
  unsigned InsnClass = MID->getSchedClass();
  const llvm::InstrStage *IS = InstrItins->beginStage(InsnClass);
  unsigned FuncUnits = IS->getUnits();
  UnsignPair StateTrans = UnsignPair(CurrentState, FuncUnits);
  ReadTable(CurrentState);
  return (CachedTable.count(StateTrans) != 0);
}


// reserveResources - Reserve the resources occupied by a MCInstrDesc and
// change the current state to reflect that change.
void DFAPacketizer::reserveResources(const llvm::MCInstrDesc *MID) {
  unsigned InsnClass = MID->getSchedClass();
  const llvm::InstrStage *IS = InstrItins->beginStage(InsnClass);
  unsigned FuncUnits = IS->getUnits();
  UnsignPair StateTrans = UnsignPair(CurrentState, FuncUnits);
  ReadTable(CurrentState);
  assert(CachedTable.count(StateTrans) != 0);
  CurrentState = CachedTable[StateTrans];
}


// canReserveResources - Check if the resources occupied by a machine
// instruction are available in the current state.
bool DFAPacketizer::canReserveResources(llvm::MachineInstr *MI) {
  const llvm::MCInstrDesc &MID = MI->getDesc();
  return canReserveResources(&MID);
}

// reserveResources - Reserve the resources occupied by a machine
// instruction and change the current state to reflect that change.
void DFAPacketizer::reserveResources(llvm::MachineInstr *MI) {
  const llvm::MCInstrDesc &MID = MI->getDesc();
  reserveResources(&MID);
}
