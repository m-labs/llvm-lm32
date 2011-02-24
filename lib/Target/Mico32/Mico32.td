//===- Mico32.td - Describe the Mico32 Target Machine ------*- tblgen -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source 
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This is the top level entry point for the Mico32 target.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Subtarget Features. 
//===----------------------------------------------------------------------===//
//FIXME: want to add invoke instruction and alternate instruction encoding.
//def FeatureZ10 : SubtargetFeature<"z10", "HasZ10Insts", "true",
//                                  "Support Z10 instructions">;

//===----------------------------------------------------------------------===//
// Mico32 supported processors.
//===----------------------------------------------------------------------===//
//class Proc<string Name, list<SubtargetFeature> Features>
// : Processor<Name, NoItineraries, Features>;
//
//def : Proc<"z9",  []>;
//def : Proc<"z10", [FeatureZ10]>;

def : Processor< "mico32", NoItineraries, []>;


//===----------------------------------------------------------------------===//
//Register File, Calling Convention, and Instruction Descriptions 
//===----------------------------------------------------------------------===//

include "Mico32RegisterInfo.td"
include "Mico32CallingConv.td"
include "Mico32InstrInfo.td"

def Mico32InstrInfo : InstrInfo {
} 

//===----------------------------------------------------------------------===//
// Target Declaration
//===----------------------------------------------------------------------===//

def Mico32 : Target {
  let InstructionSet = Mico32InstrInfo;
}

