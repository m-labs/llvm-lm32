//===- LM32.td - Describe the LM32 Target Machine -------------*- tblgen -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source 
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This is the top level entry point for the LM32 target.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Register File, Calling Conv, Instruction Descriptions
//===----------------------------------------------------------------------===//

include "LM32RegisterInfo.td"
include "LM32Schedule.td"
include "LM32InstrInfo.td"
include "LM32CallingConv.td"

def LM32InstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// Subtarget Features. 
//===----------------------------------------------------------------------===//
//FIXME: added for the (currently) undocumented DIV/MOD instructions
def FeatureSDIV : 
  SubtargetFeature<"sdiv", "HasSDIV", "false",
                   "enable undocumented div 0x27 and mod 0x35 instructions - currently does nothing">;

def FeatureDIV : 
  SubtargetFeature<"div", "HasDIV", "true",
                   "enable divu and modu instructions.">;

def FeatureMUL : 
  SubtargetFeature<"mul", "HasMUL", "true",
                   "enable mul and muli instructions.">;

def FeatureBarrel : 
  SubtargetFeature<"barrel", "HasBarrel", "true",
                   "Enable barrel shifter instructions">;

def FeatureSPBias : 
  SubtargetFeature<"nospbias", "HasSPBias", "false",
                   "SP points to last used 32 bit word on stack (not ABI compilant)">;

//===----------------------------------------------------------------------===//
// LM32 supported processors.
//===----------------------------------------------------------------------===//

def : Processor< "lm32", LM32Itineraries, [FeatureDIV, FeatureMUL,
                                           FeatureBarrel, FeatureSPBias]>;


//===----------------------------------------------------------------------===//
// Use an MC assembly printer
//===----------------------------------------------------------------------===//

//def LM32AsmWriter : AsmWriter {
//  string AsmWriterClassName  = "InstPrinter";
//  bit isMCAsmWriter = 1;
//}

//===----------------------------------------------------------------------===//
// Target Declaration
//===----------------------------------------------------------------------===//

def LM32 : Target {
  let InstructionSet = LM32InstrInfo;
//  let AssemblyWriters = [LM32AsmWriter];
}

