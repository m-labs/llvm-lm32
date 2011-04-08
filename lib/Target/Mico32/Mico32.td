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
include "Mico32RegisterInfo.td"
include "Mico32Schedule.td"
include "Mico32CallingConv.td"
include "Mico32InstrInfo.td"

//===----------------------------------------------------------------------===//
// Subtarget Features. 
//===----------------------------------------------------------------------===//
//FIXME: added for the (currently) undocumented DIV/MOD instructions
def FeatureSDIV : 
  SubtargetFeature<"sdiv", "IsSDIVenabled", "false",
                   "enable undocumented div 0x27 and mod 0x35 instructions - currently does nothing">;

def FeatureDIV : 
  SubtargetFeature<"div", "IsDIVenabled", "true",
                   "enable divu and modu instructions.">;

def FeatureMUL : 
  SubtargetFeature<"mul", "IsMULenabled", "true",
                   "enable mul and muli instructions.">;


//===----------------------------------------------------------------------===//
// Mico32 supported processors.
//===----------------------------------------------------------------------===//
def : Processor< "mico32", Mico32Itineraries, []>;


//===----------------------------------------------------------------------===//
// Register File, Calling Convention, and Instruction Descriptions 
//===----------------------------------------------------------------------===//


def Mico32InstrInfo : InstrInfo {
} 

//===----------------------------------------------------------------------===//
// Use an MC assembly printer
//===----------------------------------------------------------------------===//

def Mico32AsmWriter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  bit isMCAsmWriter = 1;
}

//===----------------------------------------------------------------------===//
// Target Declaration
//===----------------------------------------------------------------------===//

def Mico32 : Target {
  let InstructionSet = Mico32InstrInfo;
  let AssemblyWriters = [Mico32AsmWriter];
}

