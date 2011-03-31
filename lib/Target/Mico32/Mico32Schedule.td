//===- Mico32Schedule.td - Mico32 Scheduling Definitions --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Mico32 Functional Units
//===----------------------------------------------------------------------===//
def ALU	: FuncUnit;

//===----------------------------------------------------------------------===//
// Instruction Itinerary classes used for Mico32 
//===----------------------------------------------------------------------===//
//                                 // result:issue
def IIAlu       : InstrItinClass;  // 1 : 1
def IIStore     : InstrItinClass;  // X : 1
def IILoad      : InstrItinClass;  // 3 : 1
def IIBranch    : InstrItinClass;  // X : 1 or 4
def IICall      : InstrItinClass;  // X : 4
def IIShift     : InstrItinClass;  // 2 : 1
def IICmp       : InstrItinClass;  // 2 : 1
def IIDiv       : InstrItinClass;  // 34 : 34
def IIMul       : InstrItinClass;  // 3 : 1

//===----------------------------------------------------------------------===//
// Mico32 Generic instruction itineraries.
//===----------------------------------------------------------------------===//
// FIXME: This is only a first pass to get things compiling.
def Mico32Itineraries : ProcessorItineraries<
  [ALU], [], [
  InstrItinData<IIAlu              , [InstrStage<1,  [ALU]>]>,
  InstrItinData<IIStore            , [InstrStage<1,  [ALU]>]>,
  InstrItinData<IILoad             , [InstrStage<3,  [ALU]>]>,
  InstrItinData<IIBranch           , [InstrStage<1,  [ALU]>]>,
  InstrItinData<IICall             , [InstrStage<4,  [ALU]>]>,
  InstrItinData<IIShift            , [InstrStage<2,  [ALU]>]>,
  InstrItinData<IICmp              , [InstrStage<2,  [ALU]>]>,
  InstrItinData<IIDiv              , [InstrStage<34, [ALU]>]>,
  InstrItinData<IIMul              , [InstrStage<3,  [ALU]>]>
]>;
