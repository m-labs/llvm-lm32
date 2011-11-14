//===------- Mico32.td - Calling Conventions for Mico32 --*- C++ --------*-===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// This describes the calling conventions for the Mico32 architecture.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Mico32 Calling Conventions
//===----------------------------------------------------------------------===//

// Note it looks like LLVM supports passing or return stucture arguments
// directly in registers.  
// http://nondot.org/sabre/LLVMNotes/
// from the description of this feature at:
// http://nondot.org/sabre/LLVMNotes/FirstClassAggregates.txt
// it sounds like this should "just work".... we'll see.
// See also: http://nondot.org/sabre/LLVMNotes/CustomCallingConventions.txt


def CC_Mico32 : CallingConv<[
  // Promote i8/i16 arguments to i32.
  CCIfType<[i8, i16], CCPromoteToType<i32>>,

  // Integer and float arguments are passed in integer registers.
  CCIfType<[i32, f32], CCAssignToReg<[R1, R2, R3, R4, R5, R6, R7, R8]>>,

  // Integer values and floats get stored in stack slots that are 4 bytes in
  // size and 4-byte aligned.
  CCIfType<[i32, f32], CCAssignToStack<4, 4>>

]>;

// Structure returns "sret" are done by the compiler and labeled as such 
// in LLVM IR.  Unsupported types should get handled as structures.  Target
// dependent codegen should not have to do anything special to handle sret.

// Note the return registers (and caller saves) should 
// be "Defs" in the call instruction in Mico32InstrInfo.td.
def RetCC_Mico32 : CallingConv<[
  // i32 and f32 are returned in registers R1, R2.
  CCIfType<[i32, f32], CCAssignToReg<[R1, R2]>>
]>;

