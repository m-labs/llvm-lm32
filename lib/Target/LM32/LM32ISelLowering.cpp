
//===-- LM32ISelLowering.cpp - LM32 DAG Lowering Implementation -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that LM32 uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "lm32-lower"
#include "LM32ISelLowering.h"
#include "LM32MachineFunctionInfo.h"
#include "LM32TargetMachine.h"
#include "LM32TargetObjectFile.h"
#include "LM32Subtarget.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Intrinsics.h"
#include "llvm/CallingConv.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

const char *LM32TargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
    case LM32ISD::JmpLink    : return "LM32ISD::JmpLink";
    case LM32ISD::GPRel      : return "LM32ISD::GPRel";
    case LM32ISD::ICmp       : return "LM32ISD::ICmp";
    case LM32ISD::RetFlag    : return "LM32ISD::RetFlag";
    case LM32ISD::Select_CC  : return "LM32ISD::Select_CC";
    case LM32ISD::Hi         : return "LM32ISD::Hi";
    case LM32ISD::Lo         : return "LM32ISD::Lo";
    default                    : return NULL;
  }
}

LM32TargetLowering::LM32TargetLowering(LM32TargetMachine &TM)
  : TargetLowering(TM, new LM32TargetObjectFile()) {
  Subtarget = &TM.getSubtarget<LM32Subtarget>();

  // LM32 does not have i1 type, so use i32 for
  // setcc operations results (slt, sgt, ...).
  setBooleanContents(ZeroOrOneBooleanContent);

  // Set up the integer register class
  // See MBlaze for conditional floating point setup if we add that.
  addRegisterClass(MVT::i32, &LM32::GPRRegClass);

  // Load extented operations for i1 types must be promoted
  setLoadExtAction(ISD::EXTLOAD,  MVT::i1,  Promote);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::i1,  Promote);
  setLoadExtAction(ISD::SEXTLOAD, MVT::i1,  Promote);

  // Sign extended loads must be expanded
// FIXME: FIXED: are supported
//  setLoadExtAction(ISD::SEXTLOAD, MVT::i8, Expand);
//  setLoadExtAction(ISD::SEXTLOAD, MVT::i16, Expand);

  // There's no extended adds or subtracts.
  setOperationAction(ISD::ADDC, MVT::i32, Expand);
  setOperationAction(ISD::ADDE, MVT::i32, Expand);
  setOperationAction(ISD::SUBC, MVT::i32, Expand);
  setOperationAction(ISD::SUBE, MVT::i32, Expand);

  // Check if unsigned div/mod are enabled.
  if (!Subtarget->hasDIV()) {
    setOperationAction(ISD::UDIV, MVT::i32, Expand);
    setOperationAction(ISD::UREM, MVT::i32, Expand);
  }

  // Check if signed div/mod are enabled.
  if (!Subtarget->hasSDIV()) {
    setOperationAction(ISD::SDIV, MVT::i32, Expand);
    setOperationAction(ISD::SREM, MVT::i32, Expand);
  }

  // LM32 doesn't have a DIV with REM instruction.
  setOperationAction(ISD::UDIVREM, MVT::i32, Expand);
  setOperationAction(ISD::SDIVREM, MVT::i32, Expand);

  // If the processor doesn't support multiply then expand it
  if (!Subtarget->hasMUL()) {
    setOperationAction(ISD::MUL, MVT::i32, Expand);
  }

  // LM32 doesn't support 64-bit multiply.
  setOperationAction(ISD::MULHS, MVT::i32, Expand);
  setOperationAction(ISD::MULHS, MVT::i64, Expand);
  setOperationAction(ISD::MULHU, MVT::i32, Expand);
  setOperationAction(ISD::MULHU, MVT::i64, Expand);

  // Expand unsupported conversions
  setOperationAction(ISD::BITCAST, MVT::f32, Expand);
  setOperationAction(ISD::BITCAST, MVT::i32, Expand);

  // Expand SELECT_CC
//FIXME: need to implement branches
  setOperationAction(ISD::SELECT_CC, MVT::Other, Expand);

  // LM32 doesn't have MUL_LOHI
  setOperationAction(ISD::SMUL_LOHI, MVT::i32, Expand);
  setOperationAction(ISD::UMUL_LOHI, MVT::i32, Expand);
  setOperationAction(ISD::SMUL_LOHI, MVT::i64, Expand);
  setOperationAction(ISD::UMUL_LOHI, MVT::i64, Expand);

//FIXME: WHAT is this?
  // MBlaze: Used by legalize types to correctly generate the setcc result.
  // Without this, every float setcc comes with a AND/OR with the result,
  // we don't want this, since the fpcmp result goes to a flag register,
  // which is used implicitly by brcond and select operations.
  AddPromotedToType(ISD::SETCC, MVT::i1, MVT::i32);
//  AddPromotedToType(ISD::SELECT, MVT::i1, MVT::i32);
//  AddPromotedToType(ISD::SELECT_CC, MVT::i1, MVT::i32);

  setOperationAction(ISD::SETCC,            MVT::i64, Expand);
  setOperationAction(ISD::SETCC,            MVT::f32, Expand);
  setOperationAction(ISD::SETCC,            MVT::f64, Expand);


  // LM32 Custom Operations
  // Custom legalize GlobalAddress nodes into LO/HI parts.
  setOperationAction(ISD::GlobalAddress,      MVT::i32,   Custom);
  setOperationAction(ISD::GlobalTLSAddress,   MVT::i32,   Custom);
  setOperationAction(ISD::ConstantPool,       MVT::i32,   Custom);

  // Variable Argument support
  // Use custom implementation for VASTART.
  // Use the default implementation for VAARG, VACOPY, VAEND.
  setOperationAction(ISD::VASTART,            MVT::Other, Custom);
  setOperationAction(ISD::VAEND,              MVT::Other, Expand);
  setOperationAction(ISD::VAARG,              MVT::Other, Expand);
  setOperationAction(ISD::VACOPY,             MVT::Other, Expand);


  // LM32 does not have jump table branches...
  setOperationAction(ISD::BR_JT,              MVT::Other, Expand);
  // ... so we have to lower the loads from the jump table.
  setOperationAction(ISD::JumpTable,          MVT::i32,   Custom);
  // Expand BR_CC to BRCOND.
  setOperationAction(ISD::BR_CC,              MVT::Other, Expand);

  // Operations not directly supported by LM32.
  setOperationAction(ISD::SIGN_EXTEND_INREG,  MVT::i1,    Expand);
  setOperationAction(ISD::SHL_PARTS,          MVT::i32,   Expand);
  setOperationAction(ISD::SRA_PARTS,          MVT::i32,   Expand);
  setOperationAction(ISD::SRL_PARTS,          MVT::i32,   Expand);
  setOperationAction(ISD::MEMBARRIER,         MVT::Other, Expand);
  setOperationAction(ISD::ATOMIC_FENCE,       MVT::Other, Expand);
  setOperationAction(ISD::FP_ROUND,           MVT::f32,   Expand);
  setOperationAction(ISD::FP_ROUND,           MVT::f64,   Expand);
  setOperationAction(ISD::CTPOP,              MVT::i32,   Expand);
  setOperationAction(ISD::CTTZ,               MVT::i32,   Expand);
  setOperationAction(ISD::CTLZ,               MVT::i32,   Expand);
  setOperationAction(ISD::CTTZ_ZERO_UNDEF,    MVT::i32,   Expand);
  setOperationAction(ISD::CTLZ_ZERO_UNDEF,    MVT::i32,   Expand);

// FIXME: don't we have ROTL ROTR?
  setOperationAction(ISD::ROTL , MVT::i32, Expand);
  setOperationAction(ISD::ROTR , MVT::i32, Expand);
  setOperationAction(ISD::BSWAP, MVT::i32, Expand);

 // Floating point operations which are not supported
  setOperationAction(ISD::FREM,       MVT::f32, Expand);
  setOperationAction(ISD::UINT_TO_FP, MVT::i8,  Expand);
  setOperationAction(ISD::UINT_TO_FP, MVT::i16, Expand);
  setOperationAction(ISD::UINT_TO_FP, MVT::i32, Expand);
  setOperationAction(ISD::FP_TO_UINT, MVT::i32, Expand);
  setOperationAction(ISD::FP_ROUND,   MVT::f32, Expand);
  setOperationAction(ISD::FP_ROUND,   MVT::f64, Expand);
  setOperationAction(ISD::FCOPYSIGN,  MVT::f32, Expand);
  setOperationAction(ISD::FCOPYSIGN,  MVT::f64, Expand);
  setOperationAction(ISD::FSUB,       MVT::f32, Expand);
  setOperationAction(ISD::FSIN,       MVT::f32, Expand);
  setOperationAction(ISD::FCOS,       MVT::f32, Expand);
  setOperationAction(ISD::FPOWI,      MVT::f32, Expand);
  setOperationAction(ISD::FPOW,       MVT::f32, Expand);
  setOperationAction(ISD::FPOW,       MVT::f64, Expand);
  setOperationAction(ISD::FLOG,       MVT::f32, Expand);
  setOperationAction(ISD::FLOG2,      MVT::f32, Expand);
  setOperationAction(ISD::FLOG10,     MVT::f32, Expand);
  setOperationAction(ISD::FEXP,       MVT::f32, Expand);
  setOperationAction(ISD::FSQRT,      MVT::f64, Expand);
  setOperationAction(ISD::FSQRT,      MVT::f32, Expand);


  // We don't have line number support yet.
  // EH_LABEL - Represents a label in mid basic block used to track
  // locations needed for debug and exception handling tables.  These nodes
  // take a chain as input and return a chain.
// FIXME:  what does this do?
  setOperationAction(ISD::EH_LABEL,          MVT::Other, Expand);

  // Use the default for now
  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i32,  Expand);
  setOperationAction(ISD::STACKSAVE,         MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE,      MVT::Other, Expand);

  setStackPointerRegisterToSaveRestore(LM32::RSP);

  computeRegisterProperties();
}

EVT LM32TargetLowering::getSetCCResultType(EVT VT) const {
  return MVT::i32;
}

/// getFunctionAlignment - Return the Log2 alignment of this function.
unsigned LM32TargetLowering::getFunctionAlignment(const Function *) const {
  return 2;
}

SDValue LM32TargetLowering::LowerOperation(SDValue Op,
                                             SelectionDAG &DAG) const {
  switch (Op.getOpcode())
  {
    case ISD::ConstantPool:       return LowerConstantPool(Op, DAG);
    case ISD::GlobalAddress:      return LowerGlobalAddress(Op, DAG);
    case ISD::GlobalTLSAddress:   return LowerGlobalTLSAddress(Op, DAG);
    case ISD::JumpTable:          return LowerJumpTable(Op, DAG);
    case ISD::SELECT_CC:          return LowerSELECT_CC(Op, DAG);
    case ISD::VASTART:            return LowerVASTART(Op, DAG);
  }
  return SDValue();
}

//===----------------------------------------------------------------------===//
//  Lower helper functions
//===----------------------------------------------------------------------===//
MachineBasicBlock*
LM32TargetLowering::EmitInstrWithCustomInserter(MachineInstr *MI,
                                                  MachineBasicBlock *MBB)
                                                  const {
  switch (MI->getOpcode()) {
  default: assert(false && "Unexpected instr type to insert");
/*

  case LM32::ShiftRL:
  case LM32::ShiftRA:
  case LM32::ShiftL:
    return EmitCustomShift(MI, MBB);

  case LM32::Select_FCC:
  case LM32::Select_CC:
    return EmitCustomSelect(MI, MBB);

  case LM32::CAS32:
  case LM32::SWP32:
  case LM32::LAA32:
  case LM32::LAS32:
  case LM32::LAD32:
  case LM32::LAO32:
  case LM32::LAX32:
  case LM32::LAN32:
    return EmitCustomAtomic(MI, MBB);

  case LM32::MEMBARRIER:
    // The Microblaze does not need memory barriers. Just delete the pseudo
    // instruction and finish.
    MI->eraseFromParent();
    return MBB;
*/
  }
}

MachineBasicBlock*
LM32TargetLowering::EmitCustomSelect(MachineInstr *MI,
                                       MachineBasicBlock *MBB) const {
  const TargetInstrInfo *TII = getTargetMachine().getInstrInfo();
  DebugLoc dl = MI->getDebugLoc();

  // To "insert" a SELECT_CC instruction, we actually have to insert the
  // diamond control-flow pattern.  The incoming instruction knows the
  // destination vreg to set, the condition code register to branch on, the
  // true/false values to select between, and a branch opcode to use.
  const BasicBlock *LLVM_BB = MBB->getBasicBlock();
  MachineFunction::iterator It = MBB;
  ++It;

  //  thisMBB:
  //  ...
  //   TrueVal = ...
  //   setcc r1, r2, r3
  //   bNE   r1, r0, copy1MBB
  //   fallthrough --> copy0MBB
  MachineFunction *F = MBB->getParent();
  MachineBasicBlock *flsBB = F->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *dneBB = F->CreateMachineBasicBlock(LLVM_BB);

  unsigned Opc;
  switch (MI->getOperand(4).getImm()) {
  default: llvm_unreachable("Unknown branch condition");
//FIXME:
#if 0
  case LM32CC::EQ: Opc = LM32::BEQID; break;
  case LM32CC::NE: Opc = LM32::BNEID; break;
  case LM32CC::GT: Opc = LM32::BGTID; break;
  case LM32CC::LT: Opc = LM32::BLTID; break;
  case LM32CC::GE: Opc = LM32::BGEID; break;
  case LM32CC::LE: Opc = LM32::BLEID; break;
#endif
  }

  F->insert(It, flsBB);
  F->insert(It, dneBB);

  // Transfer the remainder of MBB and its successor edges to dneBB.
  dneBB->splice(dneBB->begin(), MBB,
                llvm::next(MachineBasicBlock::iterator(MI)),
                MBB->end());
  dneBB->transferSuccessorsAndUpdatePHIs(MBB);

  MBB->addSuccessor(flsBB);
  MBB->addSuccessor(dneBB);
  flsBB->addSuccessor(dneBB);

  BuildMI(MBB, dl, TII->get(Opc))
    .addReg(MI->getOperand(3).getReg())
    .addMBB(dneBB);

  //  sinkMBB:
  //   %Result = phi [ %FalseValue, copy0MBB ], [ %TrueValue, thisMBB ]
  //  ...
  //BuildMI(dneBB, dl, TII->get(LM32::PHI), MI->getOperand(0).getReg())
  //  .addReg(MI->getOperand(1).getReg()).addMBB(flsBB)
  //  .addReg(MI->getOperand(2).getReg()).addMBB(BB);

  BuildMI(*dneBB, dneBB->begin(), dl,
          TII->get(LM32::PHI), MI->getOperand(0).getReg())
    .addReg(MI->getOperand(2).getReg()).addMBB(flsBB)
    .addReg(MI->getOperand(1).getReg()).addMBB(MBB);

  MI->eraseFromParent();   // The pseudo instruction is gone now.
  return dneBB;
}


//===----------------------------------------------------------------------===//
//  Misc Lower Operation implementation
//===----------------------------------------------------------------------===//
//

SDValue LM32TargetLowering::LowerSELECT_CC(SDValue Op,
                                             SelectionDAG &DAG) const {
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue TrueVal = Op.getOperand(2);
  SDValue FalseVal = Op.getOperand(3);
  DebugLoc dl = Op.getDebugLoc();
  unsigned Opc;

DAG.viewGraph();

  SDValue CompareFlag;
  if (LHS.getValueType() == MVT::i32) {
    Opc = LM32ISD::Select_CC;
    CompareFlag = DAG.getNode(LM32ISD::ICmp, dl, MVT::i32, LHS, RHS)
                    .getValue(1);
  } else {
    llvm_unreachable("Cannot lower select_cc with unknown type");
  }

  return DAG.getNode(Opc, dl, TrueVal.getValueType(), TrueVal, FalseVal,
                     CompareFlag);
}

// Modeled on Sparc LowerGlobalAddress.
SDValue LM32TargetLowering::
LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const
{
  const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();
  DebugLoc dl = Op.getDebugLoc();
  SDValue GA = DAG.getTargetGlobalAddress(GV, dl, MVT::i32);
  // From XCore: If it's a debug information descriptor, don't mess with it.
//FIXME:  The following is from xcore but doesn't appear to work there either.
// Once that is fixed we can uncomment this:
// the following can trigger this:
//   int puts(const char * format);
//   int main(int argc, char ** argv) { puts("hello world\n"); return(5); }
#if 0
  if (DAG.isVerifiedDebugInfoDesc(Op))
    return GA;
#endif
  SDValue Hi = DAG.getNode(LM32ISD::Hi, dl, MVT::i32, GA);
  SDValue Lo = DAG.getNode(LM32ISD::Lo, dl, MVT::i32, GA);
  /* FIXME: revisit this and see if it gets optimized:
  // The goal here was to incorporate the "ADD" into the
  // LD but it doesn't work since Monarch:Lo is unsigned
  // and ADDI expectes a signed operand.
  // See MonarchISelDAGToDAG.cpp (MonarchDAGToDAGISel::SelectADDRri)
  // for a similar ADD optimiztion.
  //// I believe the order of Hi/Lo is important.  I think
  //// the Hi should be done first, then the Lo can be added in
  //// as an offset in a future ST or LD (or other) instruction
  //// and the ADD node we're creating here removed.
  //// Therefore the Hi part should be loaded arithmetically (@ha)
  //// to allow the offset to be added (instead of or'ed).
  //// SelectADDRri it looks for MonarchISD::Lo and makes
  //// this optimization (although it looks in both operands).
  //
  // Note that LM32 assembler does not have an @ha equivalent.
  // The M32R "shigh" GAS directive is what we need.
  */
  //return DAG.getNode(ISD::ADD, dl, MVT::i32, Lo, Hi);
  return DAG.getNode(ISD::OR, dl, MVT::i32, Lo, Hi);
}


SDValue LM32TargetLowering::
LowerGlobalTLSAddress(SDValue Op, SelectionDAG &DAG) const {
  llvm_unreachable("TLS not implemented for LM32.");
  return SDValue(); // Not reached
}

// Derived from PPC.
SDValue LM32TargetLowering::
LowerJumpTable(SDValue Op, SelectionDAG &DAG) const {
  DEBUG(assert(Op.getValueType() == MVT::i32 && "pointers should be 32 bits."));
  DebugLoc dl = Op.getDebugLoc();
  JumpTableSDNode *JT = cast<JumpTableSDNode>(Op);
  SDValue JTI = DAG.getTargetJumpTable(JT->getIndex(), MVT::i32);
  SDValue Hi = DAG.getNode(LM32ISD::Hi, dl, MVT::i32, JTI);
  SDValue Lo = DAG.getNode(LM32ISD::Lo, dl, MVT::i32, JTI);

  // We don't support non-static relo models yet.
  if (DAG.getTarget().getRelocationModel() == Reloc::Static ) {
    // Generate non-pic code that has direct accesses to the constant pool.
    // The address of the global is just (hi(&g)+lo(&g)).
    return DAG.getNode(ISD::OR, dl, MVT::i32, Lo, Hi);
  } else {
      llvm_unreachable("JumpTables are only supported in static mode");
  }
  
  return Op; // notreached
}


SDValue LM32TargetLowering::
LowerConstantPool(SDValue Op, SelectionDAG &DAG) const
{
  ConstantPoolSDNode *N = cast<ConstantPoolSDNode>(Op);
  // FIXME: there isn't really any debug info here
  DebugLoc dl = Op.getDebugLoc();
  const Constant *C = N->getConstVal();
  SDValue CP = DAG.getTargetConstantPool(C, MVT::i32, N->getAlignment());
  SDValue Hi = DAG.getNode(LM32ISD::Hi, dl, MVT::i32, CP);
  SDValue Lo = DAG.getNode(LM32ISD::Lo, dl, MVT::i32, CP);
  // See note above in LowerGlobalAddress about operands.
  return DAG.getNode(ISD::OR, dl, MVT::i32, Lo, Hi);
  //return DAG.getNode(ISD::ADD, dl, MVT::i32, Lo, Hi);
}

SDValue LM32TargetLowering::LowerVASTART(SDValue Op,
                                           SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  LM32FunctionInfo *FuncInfo = MF.getInfo<LM32FunctionInfo>();

  DebugLoc dl = Op.getDebugLoc();
  SDValue FI = DAG.getFrameIndex(FuncInfo->getVarArgsFrameIndex(),
                                 getPointerTy());

  // vastart just stores the address of the VarArgsFrameIndex slot into the
  // memory location argument.
  const Value *SV = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();
  return DAG.getStore(Op.getOperand(0), dl, FI, Op.getOperand(1),
                      MachinePointerInfo(SV),
                      false, false, 0);
}

//===----------------------------------------------------------------------===//
//                      Calling Convention Implementation
//
//  The lower operations present on calling convention works on this order:
//      LowerCall (virt regs --> phys regs, virt regs --> stack) 
//      LowerFormalArguments (phys --> virt regs, stack --> virt regs)
//      LowerReturn (virt regs --> phys regs)
//      LowerCall (phys regs --> virt regs)
//
//===----------------------------------------------------------------------===//

#include "LM32GenCallingConv.inc"

//===----------------------------------------------------------------------===//
//                  Call Calling Convention Implementation
//===----------------------------------------------------------------------===//
//	 This is modeled after XCore code with MIPS and others influences.
//         For XCore ABI see https://www.xmos.com/system/files/abi87.pdf
//              https://www.xmos.com/support/documentation/software
//   For mips eabi see http://www.cygwin.com/ml/binutils/2003-06/msg00436.html
//     Elements may have been used from SparcTargetLowering::LowerArguments.
//===----------------------------------------------------------------------===//
/// Monarch call implementation
/// LowerCall - This hook must be implemented to lower calls into the
/// the specified DAG. The outgoing arguments to the call are described
/// by the OutVals array, and the values to be returned by the call are
/// described by the Ins array. The implementation should fill in the
/// InVals array with legal-type return values from the call, and return
/// the resulting token chain value.
///
/// The isTailCall flag here is normative. If it is true, the
/// implementation must emit a tail call. The
/// IsEligibleForTailCallOptimization hook should be used to catch
/// cases that cannot be handled.
///

/// LowerCall - functions arguments are copied from virtual regs to
/// (physical regs)/(stack frame), CALLSEQ_START and CALLSEQ_END are emitted.
/// TODO: isVarArg, isTailCall.
SDValue
LM32TargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                               SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG                     = CLI.DAG;
  DebugLoc &dl                          = CLI.DL;
  SmallVector<ISD::OutputArg, 32> &Outs = CLI.Outs;
  SmallVector<SDValue, 32> &OutVals     = CLI.OutVals;
  SmallVector<ISD::InputArg, 32> &Ins   = CLI.Ins;
  SDValue Chain                         = CLI.Chain;
  SDValue Callee                        = CLI.Callee;
  bool &isTailCall                      = CLI.IsTailCall;
  CallingConv::ID CallConv              = CLI.CallConv;
  bool isVarArg                         = CLI.IsVarArg;

  // For now, only CallingConv::C implemented
  if (CallConv != CallingConv::Fast && CallConv != CallingConv::C)
    llvm_unreachable("Unsupported calling convention");

  // LM32 does not yet support tail call optimization
  isTailCall = false;

  // Analyze operands of the call, assigning locations to each operand.
//FIXME: Check this 16 argument number
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeCallOperands(Outs, CC_LM32);

  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NumBytes = CCInfo.getNextStackOffset();

  // Comment from ARM: Adjust the stack pointer for the new arguments...
  // These operations are automatically eliminated by the prolog/epilog pass
  Chain = DAG.getCALLSEQ_START(Chain, DAG.getIntPtrConstant(NumBytes, true));


  // With EABI is it possible to have 16 args on registers.
//FIXME: Check this 16 register argument number
//FIXME: Check this 8 stack argument number
  SmallVector<std::pair<unsigned, SDValue>, 16> RegsToPass;
  SmallVector<SDValue, 8> MemOpChains;

  // Walk the register/memloc assignments, inserting copies/loads.
  // This was based on Sparc but the Sparc code has been updated.
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    SDValue Arg = OutVals[i];

    // Promote the value if needed.
    switch (VA.getLocInfo()) {
      default: llvm_unreachable("Unknown loc info!");
      case CCValAssign::Full: break;
      case CCValAssign::SExt:
        Arg = DAG.getNode(ISD::SIGN_EXTEND, dl, VA.getLocVT(), Arg);
        break;
      case CCValAssign::ZExt:
        Arg = DAG.getNode(ISD::ZERO_EXTEND, dl, VA.getLocVT(), Arg);
        break;
      case CCValAssign::AExt:
        Arg = DAG.getNode(ISD::ANY_EXTEND, dl, VA.getLocVT(), Arg);
        break;
    }
    
    // Arguments that can be passed on register must be kept at 
    // RegsToPass vector
    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    } else {
      assert(VA.isMemLoc());

      // Create a store off the stack pointer for this argument.
      SDValue StackPtr = DAG.getRegister(LM32::RSP, MVT::i32);
      int offset = VA.getLocMemOffset();
      offset += Subtarget->hasSPBias()? 4 : 0; 
      SDValue PtrOff = DAG.getIntPtrConstant(offset);
      PtrOff = DAG.getNode(ISD::ADD, dl, MVT::i32, StackPtr, PtrOff);
      MemOpChains.push_back(DAG.getStore(Chain, dl, Arg, PtrOff, 
                                         MachinePointerInfo(),
                                         false, false, 0));
    }
  }

  // Transform all store nodes into one single node because
  // all store nodes are independent of each other.
  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, 
                        &MemOpChains[0], MemOpChains.size());

  // Build a sequence of copy-to-reg nodes chained together with token 
  // chain and flag operands which copy the outgoing args into registers.
  // The InFlag in necessary since all emited instructions must be
  // stuck together.
  SDValue InFlag;
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    Chain = DAG.getCopyToReg(Chain, dl, RegsToPass[i].first, 
                             RegsToPass[i].second, InFlag);
    InFlag = Chain.getValue(1);
  }

  // If the callee is a GlobalAddress node (quite common, every direct call is)
  // turn it into a TargetGlobalAddress node so that legalize doesn't hack it.
  // Likewise ExternalSymbol -> TargetExternalSymbol.
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), dl, getPointerTy());
  else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), getPointerTy());

  // LM32BranchLink = #chain, #target_address, #opt_in_flags...
  //             = Chain, Callee, Reg#1, Reg#2, ...  
  //
  // Returns a chain & a flag for retval copy to use.
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  SmallVector<SDValue, 16> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  // Add argument registers to the end of the list so that they are 
  // known live into the call.
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    assert (i < 13);
    Ops.push_back(DAG.getRegister(RegsToPass[i].first,
                                  RegsToPass[i].second.getValueType()));
  }

  if (InFlag.getNode())
    Ops.push_back(InFlag);

  Chain  = DAG.getNode(LM32ISD::JmpLink, dl, NodeTys, &Ops[0], Ops.size());
  InFlag = Chain.getValue(1);

  // Create the CALLSEQ_END node.
  Chain = DAG.getCALLSEQ_END(Chain,
                             DAG.getConstant(NumBytes, getPointerTy(), true),
                             DAG.getConstant(0, getPointerTy(), true),
                             InFlag);
  InFlag = Chain.getValue(1);

  // Handle result values, copying them out of physregs into vregs that we
  // return.
  return LowerCallResult(Chain, InFlag, CallConv, isVarArg, 
                         Ins, dl, DAG, InVals);
}

/// LowerCallResult - Lower the result values of an ISD::CALL into the
/// appropriate copies out of appropriate physical registers.  This assumes that
/// Chain/InFlag are the input chain/flag to use, and that TheCall is the call
/// being lowered. Returns a SDNode with the same number of values as the 
/// ISD::CALL.
// From Mips.
SDValue LM32TargetLowering::
LowerCallResult(SDValue Chain, SDValue InFlag, CallingConv::ID CallConv,
                bool isVarArg, const SmallVectorImpl<ISD::InputArg> &Ins,
                DebugLoc dl, SelectionDAG &DAG,
                SmallVectorImpl<SDValue> &InVals) const {
  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), RVLocs, *DAG.getContext());

  CCInfo.AnalyzeCallResult(Ins, RetCC_LM32);

  // Copy all of the result registers out of their specified physreg.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    Chain = DAG.getCopyFromReg(Chain, dl, RVLocs[i].getLocReg(),
                               RVLocs[i].getValVT(), InFlag).getValue(1);
    InFlag = Chain.getValue(2);
    InVals.push_back(Chain.getValue(0));
  }

  return Chain;
}

//===----------------------------------------------------------------------===//
//             Formal Arguments Calling Convention Implementation
//===----------------------------------------------------------------------===//

/// LowerFormalArguments - transform physical registers into
/// virtual registers and generate load operations for
/// arguments places on the stack.
/// TODO: isVarArg, structure return
/// Based on Mips.
/// Note: same as Monarch
SDValue LM32TargetLowering::
LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                     const SmallVectorImpl<ISD::InputArg> &Ins,
                     DebugLoc dl, SelectionDAG &DAG,
                     SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo *MFI = MF.getFrameInfo();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();
  LM32FunctionInfo *LM32FI = MF.getInfo<LM32FunctionInfo>();
  LM32FI->setVarArgsFrameIndex(0);

  // Keep track of the last register used for arguments and the last stack offset
  // used so we can find the first varargs argument.
  unsigned ArgRegEnd = LM32::R0;
  int nextLocMemOffset = 0;

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeFormalArguments(Ins, CC_LM32);

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];

    // Arguments stored on registers
    if (VA.isRegLoc()) {
      MVT RegVT = VA.getLocVT();
      // We're expecting registers to increase in numbering.
      assert(ArgRegEnd < VA.getLocReg());
      ArgRegEnd = VA.getLocReg();
      unsigned VReg;

      if (RegVT == MVT::i32)
	VReg = RegInfo.createVirtualRegister(&LM32::GPRRegClass);
// FIXME: should we have f32?
//      else if (RegVT == MVT::f32)
//	  VReg = RegInfo.createVirtualRegister(&LM32::GPRRegClass);
      else
        llvm_unreachable("RegVT not supported by LowerFormalArguments");

      // Transform the arguments stored on
      // physical registers into virtual ones
      RegInfo.addLiveIn(ArgRegEnd, VReg);
      SDValue ArgValue = DAG.getCopyFromReg(Chain, dl, VReg, RegVT);

      // If this is an 8 or 16-bit value, it has been passed promoted
      // to 32 bits.  Insert an assert[sz]ext to capture this, then
      // truncate to the right size. If if is a floating point value
      // then convert to the correct type.
      if (VA.getLocInfo() != CCValAssign::Full) {
        unsigned Opcode = 0;
        if (VA.getLocInfo() == CCValAssign::SExt)
          Opcode = ISD::AssertSext;
        else if (VA.getLocInfo() == CCValAssign::ZExt)
          Opcode = ISD::AssertZext;
        if (Opcode)
          ArgValue = DAG.getNode(Opcode, dl, RegVT, ArgValue,
                                 DAG.getValueType(VA.getValVT()));
        ArgValue = DAG.getNode(ISD::TRUNCATE, dl, VA.getValVT(), ArgValue);
      }

      InVals.push_back(ArgValue);
    } else { // VA.isRegLoc()
      assert(ArgRegEnd == LM32::R8 &&
              "We should have used all argument registers");

      // sanity check
      assert(VA.isMemLoc());

      // Load the argument to a virtual register
      unsigned ArgSize = VA.getLocVT().getSizeInBits()/8;
      assert(ArgSize == 4 && "This should be a word size.");

      // Create the frame index object for this incoming parameter...
      int FI = MFI->CreateFixedObject(ArgSize, VA.getLocMemOffset(), true);
      nextLocMemOffset = VA.getLocMemOffset() + ArgSize;

      // Create load nodes to retrieve arguments from the stack
      SDValue FIN = DAG.getFrameIndex(FI, getPointerTy());
      SDValue lod = DAG.getLoad(VA.getValVT(), dl, Chain,
                                   FIN, MachinePointerInfo::getFixedStack(FI),
                                   false, false, false, 0);
    // SelectionDAGLegalize::LegalizeOp(SDValue Op), in the ISD::Load case
    // there's a test for "if (LD->getAlignment() < ABIAlignment){".  When the
    // MERGE_VALUES node is processed it causes the load to get reprocessed.
    // So we ensure the alignment is valid here instead of hanging.
    assert(((cast<LoadSDNode>(/* SDNode* */lod.getNode()))->getAlignment() == 4) && "invalid memory access alignment");
  DEBUG(errs() << "<-LowerFormalArguments-------->\n");
  DEBUG(errs() << "<--RESULT->\n");
DEBUG((cast<LoadSDNode>(/* SDNode* */lod.getNode()))->dump());
  DEBUG(errs() << "<--------->\n");
  DEBUG(errs() << "<--------->\n");

      InVals.push_back(lod);
    }
  }

  // Code derived from MBlaze:
  // To meet ABI, when VARARGS are passed on registers, the registers
  // containing variadic values must have their values written to the
  // very top if the callee stack frame. By doing this it aligns the
  // callee saved regsiters on the stack with the caller arguments
  // already on the stack. If the last non-vararg argument was placed in
  // the stack, there's no need to save any registers since all the
  // following vararg arguments are already on the stack. If an argument
  // register was already handled (indicated by ArgRegEnd) we know it
  // not a variadic argument.
  assert(LM32::R8 - 8 == LM32::R0 && LM32::R1 - 1 == LM32::R0 &&
          "Register numbers are expected to be sequential.");
  if (isVarArg ) {
    assert(ArgRegEnd < LM32::R9 && 
            "We should only be using 8 registers for arguments");
    // Check if all the variadic arguments are already on the stack or if some are
    // still in registers.
    if (ArgRegEnd == LM32::R8) {
      // The variadic arguments are already on the stack.
      // Record the frame index of the first variable argument
      // which is a value necessary to VASTART.
      int firstVarargFI = MFI->CreateFixedObject(4, nextLocMemOffset, true);
      LM32FI->setVarArgsFrameIndex(firstVarargFI);
      DEBUG(errs() << "All varargs on stack getVarArgsFrameIndex() to:" << 
                      LM32FI->getVarArgsFrameIndex() << "\n");
    } else {
      // Used to acumulate store chains.
      std::vector<SDValue> OutChains;
  
      // We'll save all argument registers not already saved on the stack.  Store
      // higher numbered register at higher address.
      unsigned Start = LM32::R8;
      unsigned End = ArgRegEnd + 1;
      
      // Start varargs at the first empty stack location.
      int varArgOffset = -4;
      int FI = 0;
      for (; Start >= End ; Start--, varArgOffset-=4) {
	// Move argument from phys reg -> virt reg
        unsigned VReg = RegInfo.createVirtualRegister(&LM32::GPRRegClass);
        RegInfo.addLiveIn(Start, VReg);
        SDValue ArgValue = DAG.getCopyFromReg(Chain, dl, VReg, MVT::i32);
  
        FI = MFI->CreateFixedObject(4, varArgOffset, true);
        SDValue StoreFI = DAG.getFrameIndex(FI, getPointerTy());
        OutChains.push_back(DAG.getStore(Chain, dl, ArgValue, StoreFI,
                                         MachinePointerInfo(),
                                         false, false, 0));
        DEBUG(errs() << "Saving vararg register:" << Start - LM32::R0 <<
                        " at FI:" << FI << "\n");
  
      }
      // Record the frame index of the first variable argument
      // which is a value necessary to VASTART.
      DEBUG(errs() << "setVarArgsFrameIndex to:" << FI << "\n");
      LM32FI->setVarArgsFrameIndex(FI);
  
      // All stores are grouped in one node to allow the matching between
      // the size of Ins and InVals. This only happens when on varg functions
      if (!OutChains.empty()) {
        OutChains.push_back(Chain);
        Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other,
                            &OutChains[0], OutChains.size());
      }
    }
  }
  return Chain;
}

//===----------------------------------------------------------------------===//
//               Return Value Calling Convention Implementation
//===----------------------------------------------------------------------===//

// Note: Same as Monarch
SDValue LM32TargetLowering::
LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
            const SmallVectorImpl<ISD::OutputArg> &Outs,
            const SmallVectorImpl<SDValue> &OutVals,
            DebugLoc dl, SelectionDAG &DAG) const {
  // CCValAssign - represent the assignment of
  // the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), RVLocs, *DAG.getContext());

  // Analize return values.
  CCInfo.AnalyzeReturn(Outs, RetCC_LM32);

  // If this is the first return lowered for this function, add
  // the regs to the liveout set for the function.
  if (DAG.getMachineFunction().getRegInfo().liveout_empty()) {
    for (unsigned i = 0; i != RVLocs.size(); ++i)
      if (RVLocs[i].isRegLoc())
        DAG.getMachineFunction().getRegInfo().addLiveOut(RVLocs[i].getLocReg());
  }

  SDValue Flag;

  // Copy the result values into the output registers.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(), OutVals[i], Flag);

    // guarantee that all emitted copies are
    // stuck together, avoiding something bad
    Flag = Chain.getValue(1);
  }

  if (Flag.getNode())
    return DAG.getNode(LM32ISD::RetFlag, dl, MVT::Other, 
                       Chain, DAG.getRegister(LM32::RRA, MVT::i32), Flag);
  else // Return Void
    return DAG.getNode(LM32ISD::RetFlag, dl, MVT::Other, 
                       Chain, DAG.getRegister(LM32::RRA, MVT::i32));
}

//===----------------------------------------------------------------------===//
//                           LM32 Inline Assembly Support
//===----------------------------------------------------------------------===//

/// getConstraintType - Given a constraint letter, return the type of
/// constraint it is for this target.
LM32TargetLowering::ConstraintType LM32TargetLowering::
getConstraintType(const std::string &Constraint) const
{
  // LM32 specific constraint
  //
  // 'd' : An address register. Equivalent to r.
  // 'y' : Equivalent to r; retained for
  //       backwards compatibility.
  // 'f' : Floating Point registers.
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
      default : break;
      case 'd':
      case 'y':
      case 'f':
        return C_RegisterClass;
        break;
    }
  }
  return TargetLowering::getConstraintType(Constraint);
}

/// Examine constraint type and operand type and determine a weight value.
/// This object must already have been set up with the operand type
/// and the current alternative constraint selected.
TargetLowering::ConstraintWeight
LM32TargetLowering::getSingleConstraintMatchWeight(
    AsmOperandInfo &info, const char *constraint) const {
  ConstraintWeight weight = CW_Invalid;
  Value *CallOperandVal = info.CallOperandVal;
    // If we don't have a value, we can't do a match,
    // but allow it at the lowest weight.
  if (CallOperandVal == NULL)
    return CW_Default;
  Type *type = CallOperandVal->getType();
  // Look at the constraint type.
  switch (*constraint) {
  default:
    weight = TargetLowering::getSingleConstraintMatchWeight(info, constraint);
    break;
  case 'd':
  case 'y':
    if (type->isIntegerTy())
      weight = CW_Register;
    break;
  case 'f':
    if (type->isFloatTy())
      weight = CW_Register;
    break;
  }
  return weight;
}

/// getRegForInlineAsmConstraint - Given a physical register constraint (e.g.
/// {edx}), return the register number and the register class for the
/// register. (not implemented)
///
/// Given a register class constraint, like 'r', if this corresponds directly
/// to an LLVM register class, return a register of 0 and the register class
/// pointer.
///
/// This should only be used for C_Register constraints.  On error,
/// this returns a register number of 0 and a null register class pointer..
std::pair<unsigned, const TargetRegisterClass*>
LM32TargetLowering::
getRegForInlineAsmConstraint(const std::string &Constraint, EVT VT) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    case 'r':
      return std::make_pair(0U, &LM32::GPRRegClass);
      // FIXME:  This was copied directly from MBLAZE:
      // TODO: These can't possibly be right, but match what was in
      // getRegClassForInlineAsmConstraint.
    case 'd':
    case 'y':
    case 'f':
      if (VT == MVT::f32)
        return std::make_pair(0U, &LM32::GPRRegClass);
    }
  }
  return TargetLowering::getRegForInlineAsmConstraint(Constraint, VT);
}

bool LM32TargetLowering::
isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const {
  // The LM32 target isn't yet aware of offsets.
  return false;
}

bool LM32TargetLowering::isFPImmLegal(const APFloat &Imm, EVT VT) const {
  return VT != MVT::f32;
}
