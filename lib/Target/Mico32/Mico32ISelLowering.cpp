//===-- Mico32ISelLowering.cpp - Mico32 DAG Lowering Implementation -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that Mico32 uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "mico32-lower"
#include "Mico32ISelLowering.h"
#include "Mico32MachineFunctionInfo.h"
#include "Mico32TargetMachine.h"
#include "Mico32TargetObjectFile.h"
#include "Mico32Subtarget.h"
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

const char *Mico32TargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
    case Mico32ISD::JmpLink    : return "Mico32ISD::JmpLink";
    case Mico32ISD::GPRel      : return "Mico32ISD::GPRel";
    case Mico32ISD::Wrap       : return "Mico32ISD::Wrap";
    case Mico32ISD::ICmp       : return "Mico32ISD::ICmp";
    case Mico32ISD::RetFlag    : return "Mico32ISD::RetFlag";
    case Mico32ISD::Select_CC  : return "Mico32ISD::Select_CC";
    default                    : return NULL;
  }
}

Mico32TargetLowering::Mico32TargetLowering(Mico32TargetMachine &TM)
  : TargetLowering(TM, new Mico32TargetObjectFile()) {
  Subtarget = &TM.getSubtarget<Mico32Subtarget>();

  // Mico32 does not have i1 type, so use i32 for
  // setcc operations results (slt, sgt, ...).
  setBooleanContents(ZeroOrOneBooleanContent);

  // Set up the integer register class
  // See MBlaze for conditional floating point setup if we add that.
  addRegisterClass(MVT::i32, Mico32::GPRRegisterClass);

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
  setOperationAction(ISD::FSIN,       MVT::f32, Expand);
  setOperationAction(ISD::FCOS,       MVT::f32, Expand);
  setOperationAction(ISD::FPOWI,      MVT::f32, Expand);
  setOperationAction(ISD::FPOW,       MVT::f32, Expand);
  setOperationAction(ISD::FLOG,       MVT::f32, Expand);
  setOperationAction(ISD::FLOG2,      MVT::f32, Expand);
  setOperationAction(ISD::FLOG10,     MVT::f32, Expand);
  setOperationAction(ISD::FEXP,       MVT::f32, Expand);

  // Load extented operations for i1 types must be promoted
  setLoadExtAction(ISD::EXTLOAD,  MVT::i1,  Promote);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::i1,  Promote);
  setLoadExtAction(ISD::SEXTLOAD, MVT::i1,  Promote);

  // Sign extended loads must be expanded
  setLoadExtAction(ISD::SEXTLOAD, MVT::i8, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::i16, Expand);

  // Check if unsigned div/mod are enabled.
  if (!Subtarget->hasDIV()) {
    setOperationAction(ISD::UDIV, MVT::i32, Expand);
    setOperationAction(ISD::UREM,    MVT::i32, Expand);
  }

  // Check if signed div/mod are enabled.
  if (!Subtarget->hasSDIV()) {
    setOperationAction(ISD::SDIV, MVT::i32, Expand);
    setOperationAction(ISD::SREM,    MVT::i32, Expand);
  }

  // Mico32 doesn't have a DIV with REM instruction.
  setOperationAction(ISD::UDIVREM, MVT::i32, Expand);
  setOperationAction(ISD::SDIVREM, MVT::i32, Expand);

  // If the processor doesn't support multiply then expand it
  if (!Subtarget->hasMUL()) {
    setOperationAction(ISD::MUL, MVT::i32, Expand);
  }

  // Mico32 doesn't support 64-bit multiply.
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

  // Mico32 doesn't have MUL_LOHI
  setOperationAction(ISD::SMUL_LOHI, MVT::i32, Expand);
  setOperationAction(ISD::UMUL_LOHI, MVT::i32, Expand);
  setOperationAction(ISD::SMUL_LOHI, MVT::i64, Expand);
  setOperationAction(ISD::UMUL_LOHI, MVT::i64, Expand);

  // Used by legalize types to correctly generate the setcc result.
  // Without this, every float setcc comes with a AND/OR with the result,
  // we don't want this, since the fpcmp result goes to a flag register,
  // which is used implicitly by brcond and select operations.
//FIXME: WHAT is this?
  AddPromotedToType(ISD::SETCC, MVT::i1, MVT::i32);
  AddPromotedToType(ISD::SELECT, MVT::i1, MVT::i32);
  AddPromotedToType(ISD::SELECT_CC, MVT::i1, MVT::i32);

  // Mico32 Custom Operations
  // Custom legalize GlobalAddress nodes into LO/HI parts.
  setOperationAction(ISD::GlobalAddress,      MVT::i32,   Custom);
  setOperationAction(ISD::GlobalTLSAddress,   MVT::i32,   Custom);
  setOperationAction(ISD::ConstantPool,       MVT::i32,   Custom);
// FIXME: need to add this
  // setOperationAction(ISD::JumpTable,          MVT::i32,   Custom);

  // Variable Argument support
  // Use custom implementation for VASTART.
  // Use the default implementation for VAARG, VACOPY, VAEND.
  setOperationAction(ISD::VASTART,            MVT::Other, Custom);
  setOperationAction(ISD::VAEND,              MVT::Other, Expand);
  setOperationAction(ISD::VAARG,              MVT::Other, Expand);
  setOperationAction(ISD::VACOPY,             MVT::Other, Expand);


  // Operations not directly supported by Mico32.
// FIXME: need to handle branches
//  setOperationAction(ISD::BR_JT,              MVT::Other, Expand);
//  setOperationAction(ISD::BR_CC,              MVT::Other, Expand);
  // Operations not directly supported by Mico32.
  setOperationAction(ISD::SIGN_EXTEND_INREG,  MVT::i1,    Expand);
  setOperationAction(ISD::ROTL,               MVT::i32,   Expand);
  setOperationAction(ISD::ROTR,               MVT::i32,   Expand);
  setOperationAction(ISD::SHL_PARTS,          MVT::i32,   Expand);
  setOperationAction(ISD::SRA_PARTS,          MVT::i32,   Expand);
  setOperationAction(ISD::SRL_PARTS,          MVT::i32,   Expand);
  setOperationAction(ISD::CTLZ,               MVT::i32,   Expand);
  setOperationAction(ISD::CTTZ,               MVT::i32,   Expand);
  setOperationAction(ISD::CTPOP,              MVT::i32,   Expand);
  setOperationAction(ISD::BSWAP,              MVT::i32,   Expand);

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

  setStackPointerRegisterToSaveRestore(Mico32::RSP);

  computeRegisterProperties();
}

MVT::SimpleValueType Mico32TargetLowering::getSetCCResultType(EVT VT) const {
  return MVT::i32;
}

/// getFunctionAlignment - Return the Log2 alignment of this function.
unsigned Mico32TargetLowering::getFunctionAlignment(const Function *) const {
  return 2;
}

SDValue Mico32TargetLowering::LowerOperation(SDValue Op,
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
Mico32TargetLowering::EmitInstrWithCustomInserter(MachineInstr *MI,
                                                  MachineBasicBlock *MBB)
                                                  const {
  switch (MI->getOpcode()) {
  default: assert(false && "Unexpected instr type to insert");
/*

  case Mico32::ShiftRL:
  case Mico32::ShiftRA:
  case Mico32::ShiftL:
    return EmitCustomShift(MI, MBB);

  case Mico32::Select_FCC:
  case Mico32::Select_CC:
    return EmitCustomSelect(MI, MBB);

  case Mico32::CAS32:
  case Mico32::SWP32:
  case Mico32::LAA32:
  case Mico32::LAS32:
  case Mico32::LAD32:
  case Mico32::LAO32:
  case Mico32::LAX32:
  case Mico32::LAN32:
    return EmitCustomAtomic(MI, MBB);

  case Mico32::MEMBARRIER:
    // The Microblaze does not need memory barriers. Just delete the pseudo
    // instruction and finish.
    MI->eraseFromParent();
    return MBB;
*/
  }
}

MachineBasicBlock*
Mico32TargetLowering::EmitCustomSelect(MachineInstr *MI,
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
  case Mico32CC::EQ: Opc = Mico32::BEQID; break;
  case Mico32CC::NE: Opc = Mico32::BNEID; break;
  case Mico32CC::GT: Opc = Mico32::BGTID; break;
  case Mico32CC::LT: Opc = Mico32::BLTID; break;
  case Mico32CC::GE: Opc = Mico32::BGEID; break;
  case Mico32CC::LE: Opc = Mico32::BLEID; break;
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
  //BuildMI(dneBB, dl, TII->get(Mico32::PHI), MI->getOperand(0).getReg())
  //  .addReg(MI->getOperand(1).getReg()).addMBB(flsBB)
  //  .addReg(MI->getOperand(2).getReg()).addMBB(BB);

  BuildMI(*dneBB, dneBB->begin(), dl,
          TII->get(Mico32::PHI), MI->getOperand(0).getReg())
    .addReg(MI->getOperand(2).getReg()).addMBB(flsBB)
    .addReg(MI->getOperand(1).getReg()).addMBB(MBB);

  MI->eraseFromParent();   // The pseudo instruction is gone now.
  return dneBB;
}


//===----------------------------------------------------------------------===//
//  Misc Lower Operation implementation
//===----------------------------------------------------------------------===//
//

SDValue Mico32TargetLowering::LowerSELECT_CC(SDValue Op,
                                             SelectionDAG &DAG) const {
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue TrueVal = Op.getOperand(2);
  SDValue FalseVal = Op.getOperand(3);
  DebugLoc dl = Op.getDebugLoc();
  unsigned Opc;

  SDValue CompareFlag;
  if (LHS.getValueType() == MVT::i32) {
    Opc = Mico32ISD::Select_CC;
    CompareFlag = DAG.getNode(Mico32ISD::ICmp, dl, MVT::i32, LHS, RHS)
                    .getValue(1);
  } else {
    llvm_unreachable("Cannot lower select_cc with unknown type");
  }

  return DAG.getNode(Opc, dl, TrueVal.getValueType(), TrueVal, FalseVal,
                     CompareFlag);
}

SDValue Mico32TargetLowering::
LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const {
  // FIXME there isn't actually debug info here
  DebugLoc dl = Op.getDebugLoc();
  const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();
  SDValue GA = DAG.getTargetGlobalAddress(GV, dl, MVT::i32);

  return DAG.getNode(Mico32ISD::Wrap, dl, MVT::i32, GA);
}

SDValue Mico32TargetLowering::
LowerGlobalTLSAddress(SDValue Op, SelectionDAG &DAG) const {
  llvm_unreachable("TLS not implemented for MicroBlaze.");
  return SDValue(); // Not reached
}

SDValue Mico32TargetLowering::
LowerJumpTable(SDValue Op, SelectionDAG &DAG) const {
  SDValue ResNode;
  SDValue HiPart;
  // FIXME there isn't actually debug info here
  DebugLoc dl = Op.getDebugLoc();

  EVT PtrVT = Op.getValueType();
  JumpTableSDNode *JT  = cast<JumpTableSDNode>(Op);

  SDValue JTI = DAG.getTargetJumpTable(JT->getIndex(), PtrVT, 0);
  return DAG.getNode(Mico32ISD::Wrap, dl, MVT::i32, JTI);
}

SDValue Mico32TargetLowering::
LowerConstantPool(SDValue Op, SelectionDAG &DAG) const {
  SDValue ResNode;
  ConstantPoolSDNode *N = cast<ConstantPoolSDNode>(Op);
  const Constant *C = N->getConstVal();
  DebugLoc dl = Op.getDebugLoc();

  SDValue CP = DAG.getTargetConstantPool(C, MVT::i32, N->getAlignment(),
                                         N->getOffset(), 0);
  return DAG.getNode(Mico32ISD::Wrap, dl, MVT::i32, CP);
}

SDValue Mico32TargetLowering::LowerVASTART(SDValue Op,
                                           SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  Mico32FunctionInfo *FuncInfo = MF.getInfo<Mico32FunctionInfo>();

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

#include "Mico32GenCallingConv.inc"

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
SDValue Mico32TargetLowering::
LowerCall(SDValue Chain, SDValue Callee, CallingConv::ID CallConv,
          bool isVarArg, bool &isTailCall,
          const SmallVectorImpl<ISD::OutputArg> &Outs,
          const SmallVectorImpl<SDValue> &OutVals,
          const SmallVectorImpl<ISD::InputArg> &Ins,
          DebugLoc dl, SelectionDAG &DAG,
          SmallVectorImpl<SDValue> &InVals) const {

  // For now, only CallingConv::C implemented
  if (CallConv != CallingConv::Fast && CallConv != CallingConv::C)
    llvm_unreachable("Unsupported calling convention");

  // Mico32 does not yet support tail call optimization
  isTailCall = false;

  // Analyze operands of the call, assigning locations to each operand.
//FIXME: Check this 16 argument number
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, getTargetMachine(),
                 ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeCallOperands(Outs, CC_Mico32);

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
      SDValue StackPtr = DAG.getRegister(Mico32::RSP, MVT::i32);
      SDValue PtrOff = DAG.getIntPtrConstant(VA.getLocMemOffset());
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
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), dl, MVT::i32);
    //Mips: Callee = DAG.getT...Address(G->getGlobal(), getPointerTy());
  else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), MVT::i32);
    // Mips: Callee = DAG.getT...lSymbol(E->getSymbol(), getPointerTy());

  // Mico32BranchLink = #chain, #target_address, #opt_in_flags...
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

  Chain  = DAG.getNode(Mico32ISD::JmpLink, dl, NodeTys, &Ops[0], Ops.size());
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
SDValue Mico32TargetLowering::
LowerCallResult(SDValue Chain, SDValue InFlag, CallingConv::ID CallConv,
                bool isVarArg, const SmallVectorImpl<ISD::InputArg> &Ins,
                DebugLoc dl, SelectionDAG &DAG,
                SmallVectorImpl<SDValue> &InVals) const {
  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, getTargetMachine(),
                 RVLocs, *DAG.getContext());

  CCInfo.AnalyzeCallResult(Ins, RetCC_Mico32);

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
SDValue Mico32TargetLowering::
LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                     const SmallVectorImpl<ISD::InputArg> &Ins,
                     DebugLoc dl, SelectionDAG &DAG,
                     SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo *MFI = MF.getFrameInfo();
  Mico32FunctionInfo *Mico32FI = MF.getInfo<Mico32FunctionInfo>();

  // Used with vargs to acumulate store chains.
  std::vector<SDValue> OutChains;

  // Keep track of the last register used for arguments
  unsigned ArgRegEnd = 0;

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, getTargetMachine(),
                 ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeFormalArguments(Ins, CC_Mico32);

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];

    // Arguments stored on registers
    if (VA.isRegLoc()) {
      MVT RegVT = VA.getLocVT();
      ArgRegEnd = VA.getLocReg();
      TargetRegisterClass *RC = 0;

      if (RegVT == MVT::i32)
        RC = Mico32::GPRRegisterClass;
// FIXME: should we have f32?
//      else if (RegVT == MVT::f32)
//        RC = Mico32::GPRRegisterClass;
      else
        llvm_unreachable("RegVT not supported by LowerFormalArguments");

      // Transform the arguments stored on
      // physical registers into virtual ones
      unsigned Reg = MF.addLiveIn(ArgRegEnd, RC);
      SDValue ArgValue = DAG.getCopyFromReg(Chain, dl, Reg, RegVT);

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
      // sanity check
      assert(VA.isMemLoc());

      // Load the argument to a virtual register
      unsigned ArgSize = VA.getLocVT().getSizeInBits()/8;
      assert(ArgSize == 4 && "This should be a word size.");
#if 0
      if (ArgSize > StackSlotSize) {
        report_fatal_error("LowerFormalArguments Unhandled argument type: " +
                         Twine((unsigned)VA.getLocVT().getSimpleVT().SimpleTy));
      }
#endif


      // Create the frame index object for this incoming parameter...
      int FI = MFI->CreateFixedObject(ArgSize, VA.getLocMemOffset(), true);

      // Create load nodes to retrieve arguments from the stack
      SDValue FIN = DAG.getFrameIndex(FI, getPointerTy());
      SDValue lod = DAG.getLoad(VA.getValVT(), dl, Chain,
                                   FIN, MachinePointerInfo::getFixedStack(FI),
                                   false, false, 0);
    // SelectionDAGLegalize::LegalizeOp(SDValue Op), in the ISD::Load case
    // there's a test for "if (LD->getAlignment() < ABIAlignment){".  When the
    // MERGE_VALUES node is processed it causes the load to get reprocessed.
    // So we ensure the alignment is valid here instead of hanging.
    assert(((cast<LoadSDNode>(/* SDNode* */lod.getNode()))->getAlignment() == 4) && "invalid memory access alignment");

      InVals.push_back(lod);
    }
  }

  // For Variable argument functions Mico32 passes all parameters on the stack.
  // The formal argument processing only processes the non-variable parameters,
  // the variable parameters have not been processed yet so 
  // CCInfo.getNextStackOffset() will give us the first variable argument.
  if (isVarArg) {
      // This will point to the next argument passed via stack.
      Mico32FI->setVarArgsFrameIndex(
                  MFI->CreateFixedObject(4, CCInfo.getNextStackOffset(), true));
  }

  return Chain;
}

//===----------------------------------------------------------------------===//
//               Return Value Calling Convention Implementation
//===----------------------------------------------------------------------===//

// Note: Same as Monarch
SDValue Mico32TargetLowering::
LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
            const SmallVectorImpl<ISD::OutputArg> &Outs,
            const SmallVectorImpl<SDValue> &OutVals,
            DebugLoc dl, SelectionDAG &DAG) const {
  // CCValAssign - represent the assignment of
  // the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, isVarArg, getTargetMachine(),
                 RVLocs, *DAG.getContext());

  // Analize return values.
  CCInfo.AnalyzeReturn(Outs, RetCC_Mico32);

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
    return DAG.getNode(Mico32ISD::RetFlag, dl, MVT::Other, 
                       Chain, DAG.getRegister(Mico32::RRA, MVT::i32), Flag);
  else // Return Void
    return DAG.getNode(Mico32ISD::RetFlag, dl, MVT::Other, 
                       Chain, DAG.getRegister(Mico32::RRA, MVT::i32));
}

//===----------------------------------------------------------------------===//
//                           Mico32 Inline Assembly Support
//===----------------------------------------------------------------------===//

/// getConstraintType - Given a constraint letter, return the type of
/// constraint it is for this target.
#if 0
Mico32TargetLowering::ConstraintType Mico32TargetLowering::
getConstraintType(const std::string &Constraint) const
{
  // Mico32 specific constrainy
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
#endif

/// Examine constraint type and operand type and determine a weight value.
/// This object must already have been set up with the operand type
/// and the current alternative constraint selected.
#if 0
TargetLowering::ConstraintWeight
Mico32TargetLowering::getSingleConstraintMatchWeight(
    AsmOperandInfo &info, const char *constraint) const {
  ConstraintWeight weight = CW_Invalid;
  Value *CallOperandVal = info.CallOperandVal;
    // If we don't have a value, we can't do a match,
    // but allow it at the lowest weight.
  if (CallOperandVal == NULL)
    return CW_Default;
  const Type *type = CallOperandVal->getType();
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
#endif

/// getRegClassForInlineAsmConstraint - Given a constraint letter (e.g. "r"),
/// return a list of registers that can be used to satisfy the constraint.
/// This should only be used for C_RegisterClass constraints.
#if 0
std::pair<unsigned, const TargetRegisterClass*> Mico32TargetLowering::
getRegForInlineAsmConstraint(const std::string &Constraint, EVT VT) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    case 'r':
      return std::make_pair(0U, Mico32::GPRRegisterClass);
    case 'f':
      if (VT == MVT::f32)
        return std::make_pair(0U, Mico32::GPRRegisterClass);
    }
  }
  return TargetLowering::getRegForInlineAsmConstraint(Constraint, VT);
}
#endif

/// Given a register class constraint, like 'r', if this corresponds directly
/// to an LLVM register class, return a register of 0 and the register class
/// pointer.
#if 0
std::vector<unsigned> Mico32TargetLowering::
getRegClassForInlineAsmConstraint(const std::string &Constraint, EVT VT) const {
  if (Constraint.size() != 1)
    return std::vector<unsigned>();

  switch (Constraint[0]) {
    default : break;
    case 'r':
    // GCC Mico32 Constraint Letters
    case 'd':
    case 'y':
    case 'f':
      return make_vector<unsigned>(
        Mico32::R3,  Mico32::R4,  Mico32::R5,  Mico32::R6,
        Mico32::R7,  Mico32::R9,  Mico32::R10, Mico32::R11,
        Mico32::R12, Mico32::R19, Mico32::R20, Mico32::R21,
        Mico32::R22, Mico32::R23, Mico32::R24, Mico32::R25,
        Mico32::R26, Mico32::RFP, Mico32::RSP, Mico32::RRA,
        Mico32::REA, Mico32::RBA, 0);
  }
  return std::vector<unsigned>();
}
#endif

#if 0
bool Mico32TargetLowering::
isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const {
  // The Mico32 target isn't yet aware of offsets.
  return false;
}
#endif

#if 0
bool Mico32TargetLowering::isFPImmLegal(const APFloat &Imm, EVT VT) const {
  return VT != MVT::f32;
}
#endif
