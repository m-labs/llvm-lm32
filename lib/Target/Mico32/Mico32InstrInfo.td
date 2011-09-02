//===- Mico32InstrInfo.td - Mico32 Instruction defs --------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Instruction format superclass
//===----------------------------------------------------------------------===//
include "Mico32InstrFormats.td"

//===----------------------------------------------------------------------===//
// Mico32 type profiles
//===----------------------------------------------------------------------===//

def SDT_Mico32CallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>]>;
def SDT_Mico32CallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;

// SDT_Mico32Call has no results, target address operand, ...
// -1 indicates variadic/arbitrary number of arguments.
def SDT_Mico32Call : SDTypeProfile<0, -1, [SDTCisVT<0, i32>]>;

//===----------------------------------------------------------------------===//
// Mico32 specific nodes
//===----------------------------------------------------------------------===//

def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_Mico32CallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;

def callseq_end   : SDNode<"ISD::CALLSEQ_END", SDT_Mico32CallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

// A mico32 return instruction SDNode.
def RetFlag       : SDNode<"Mico32ISD::RetFlag", SDTNone,
                           [SDNPHasChain, SDNPOptInGlue]>;

def Mico32Call    : SDNode<"Mico32ISD::JmpLink", SDT_Mico32Call,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue,
                            SDNPVariadic]>;

// Hi and Lo nodes are used to handle global addresses. Used in 
// Mico32ISelLowering to lower stuff like GlobalAddress, ExternalSymbol 
// static model.
// These are generated in LowerConstantPool and LowerGLobalAddress.
def Mico32Hi    : SDNode<"Mico32ISD::Hi", SDTIntUnaryOp>;
def Mico32Lo    : SDNode<"Mico32ISD::Lo", SDTIntUnaryOp>;


//===----------------------------------------------------------------------===//
// Mico32 Instruction Predicate Definitions.
//===----------------------------------------------------------------------===//
def HasBarrel    : Predicate<"Subtarget.hasBarrel()">;
//def NoBarrel     : Predicate<"!Subtarget.hasBarrel()">;
def HasDIV       : Predicate<"Subtarget.hasDIV()">;
def HasSDIV      : Predicate<"Subtarget.hasSDIV()">;
def HasMUL       : Predicate<"Subtarget.hasMUL()">;

//===----------------------------------------------------------------------===//
// Mico32 Operand, Complex Patterns and Transformations Definitions.
//===----------------------------------------------------------------------===//


// From XCore.
// Transformation function: imm/4
def div4_xform : SDNodeXForm<imm, [{
  assert(N->getZExtValue() % 4 == 0);
  return getI32Imm(N->getZExtValue()/4);
}]>;

// A 28 bit sign extened offset that is used in the instructions
// as a 26 bit _word_ offset. Used with the div4_xform SDNodeXForm
// it is converted to a 26 bit word offset. (Also modeled after XCore.)
// isInt() is from MathExtras.h.
def immSExt26ShftOff : PatLeaf<(imm),
                            [{return (isInt<28>(N->getSExtValue())) && 
                                     ((N->getSExtValue() & 0x03) == 0);}]>;


// This is from MBlaze - need to see how it works.
def Mico32MemAsmOperand : AsmOperandClass {
  let Name = "Mem";
  let SuperClasses = [];
}

// Instruction operand types
def brtarget    : Operand<OtherVT>;
def calltarget  : Operand<i32>;
def simm16      : Operand<i32> {
  let PrintMethod = "printS16ImmOperand";
}
def uimm5       : Operand<i32>;

// From SPU:
def symbolHi    : Operand<i32> {
  let PrintMethod = "printSymbolHi";
}
def symbolLo    : Operand<i32> {
  let PrintMethod = "printSymbolLo";
}



// Unsigned Operand
def uimm16      : Operand<i32> {
  let PrintMethod = "printUnsignedImm";
}

// From TargetSelectionDAG.td:
// Patterns are what are actually matched against the target-flavored
// instruction selection DAG.  Instructions defined by the target implicitly
// define patterns in most cases, but patterns can also be explicitly added when
// an operation is defined by a sequence of instructions (e.g. loading a large
// immediate value on RISC targets that do not support immediates as large as
// their GPRs).

// Extract bits 0-15 (low-end) of an immediate value.
def LO16 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant((unsigned)N->getZExtValue() & 0xffff,
                                   MVT::i32);
}]>;

// Extract bits 16-31 (high-end) of an immediate value.
// Transformation function: shift the immediate value down into the low bits.
def HI16 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant((unsigned)N->getZExtValue() >> 16, MVT::i32);
}]>;

// shift amount field must fit in 5 bits.
def immZExt5 : PatLeaf<(imm), [{return isUInt<5>(N->getZExtValue());}]>;

// immSExt16 predicate - True if the immediate fits in a 16-bit sign extended
// field.  Used by instructions like 'addi'.
def immSExt16  : PatLeaf<(imm), [{return isInt<16>(N->getSExtValue());}]>;

// immZExt16 predicate - True if the immediate fits in a 16-bit zero extended
// field.  Used by instructions like 'ori'.
def immZExt16 : PatLeaf<(imm), [{return isUInt<16>(N->getZExtValue());}], LO16>;


// From PowerPC: 
// imm16ShiftedZExt predicate - True if only bits in the top 16-bits of the
// immediate are set.  Used by instructions like 'oris'.
def imm16ShiftedZExt : PatLeaf<(imm), [{
  return (N->getZExtValue() & ~uint64_t(0xFFFF0000)) == 0;
}], HI16>;






// Addressing modes.
// Note: SelectAddrRegImm is defined in Mico32ISelDAGToDAG.cpp.
// See the maillist thread "Using frameindex in a pattern",
// "ComplexPattern in child ISel nodes",
//
// From TargetSelectionDAG.td:
// Complex patterns, e.g. X86 addressing mode, requires pattern matching code in
// C++. NumOperands is the number of operands returned by the select function;
// SelectFunc is the name of the function used to pattern match the maximum 
// pattern;
// RootNodes are the list of possible root nodes of the sub-dags to match.
// e.g. X86 addressing mode - def addr : ComplexPattern<4, "SelectAddr", [add]>;
//
// Here's my understanding - since this only applies to frameindex root nodes
// these will not be used for non-frameindex references.  Is this a problem?
// I don't know, in the general case I think not since we are limited to 
// 16bit immediate operands and in the non-frame cases will probably have to 
// use register modes anyways.
//
// Here's the 1000 foot overview:
// So i'm not sure what the "root node" does.  I'm assuming it is matched in
// select in addition to the ADDRri operands.  If it's removed the LEA doesn't
// work because no select is generated for frameindex.
//
// If the node is of the type [frameindex] and "SelectAddrRegImm" returns true
// then the pattern is matched.  SelectAddrRegImm returns the number of operands
// specified, 2 in this case, these 2 operands are then passed as the arguments
// to the node using the ADDRri pattern.
//
// Mico32 Address Mode. SDNode frameindex could possibily be a match
// since load and store instructions from stack used it.
// From MBlaze:
def ADDRri : ComplexPattern<i32, 2, "SelectAddrRegImm", [frameindex], []>;

// Address operand
// ADDRri addressing _mode_ corresponds to MEMri _operands_.
def MEMri : Operand<i32> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops GPR, simm16);
  let ParserMatchClass = Mico32MemAsmOperand;
}

def MEMrr : Operand<i32> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops GPR, GPR);
  let ParserMatchClass = Mico32MemAsmOperand;
}


//===----------------------------------------------------------------------===//
// Pseudo instructions
//===----------------------------------------------------------------------===//

// This is a call frame create  pseudo opcode.
// As stack alignment is always done with addiu, we need a 16-bit immediate
let Defs = [RSP], Uses = [RSP] in {
def ADJCALLSTACKDOWN : Mico32Pseudo<(outs), (ins simm16:$amt),
                                    "#ADJCALLSTACKDOWN $amt",
                                    [(callseq_start timm:$amt)]>;
def ADJCALLSTACKUP   : Mico32Pseudo<(outs),
                                    (ins uimm16:$amt1, simm16:$amt2),
                                    "#ADJCALLSTACKUP $amt1",
                                    [(callseq_end timm:$amt1, timm:$amt2)]>;
}

//===----------------------------------------------------------------------===//
// Arithmetic Instruction Formats
//===----------------------------------------------------------------------===//
// RR instruction
class ArithRR<bits<6> op, string instr_asm, SDNode OpNode,
              InstrItinClass itin> :
             TRR<op, (outs GPR:$dst), (ins GPR:$b, GPR:$c),
                 !strconcat(instr_asm, " $dst, $b, $c"),
                 [(set GPR:$dst, (OpNode GPR:$b, GPR:$c))], itin>;

// Fragmant version of RR instruction
class ArithFRR<bits<6> op, string instr_asm, PatFrag OpNode,
               InstrItinClass itin> :
              TRR<op, (outs GPR:$dst), (ins GPR:$b, GPR:$c),
                  !strconcat(instr_asm, " $dst, $b, $c"),
                  [(set GPR:$dst, (OpNode GPR:$b, GPR:$c))], itin>;

// RR instruction with not in pattern.
class ArithNRR<bits<6> op, string instr_asm, SDNode OpNode,
              InstrItinClass itin> :
             TRR<op, (outs GPR:$dst), (ins GPR:$b, GPR:$c),
                 !strconcat(instr_asm, " $dst, $b, $c"),
                 [(set GPR:$dst, (not (OpNode GPR:$b, GPR:$c)))], itin>;

// RI instruction with 16bit zero extended immediate.
class ArithRIz<bits<6> op, string instr_asm, SDNode OpNode,
               InstrItinClass itin> :
              TRI<op, (outs GPR:$dst), (ins GPR:$b, uimm16:$c),
                  !strconcat(instr_asm, " $dst, $b, $c"),
                  [(set GPR:$dst, (OpNode GPR:$b, immZExt16:$c))], itin>;

class ArithRIsym<bits<6> op, string instr_asm, SDNode OpNode,
                 InstrItinClass itin> :
                TRI<op, (outs GPR:$dst), (ins GPR:$b, symbolLo:$c),
                    !strconcat(instr_asm, "   $dst, $b, $c"),
                    [], itin>;

// Fragment version of RI instruction with 16bit zero extended immediate.
class ArithFRIz<bits<6> op, string instr_asm, PatFrag OpNode,
                InstrItinClass itin> :
               TRI<op, (outs GPR:$dst), (ins GPR:$b, uimm16:$c),
                   !strconcat(instr_asm, " $dst, $b, $c"),
                   [(set GPR:$dst, (OpNode GPR:$b, immZExt16:$c))], itin>;

// ArithRIz with not in pattern.
class ArithNRIz<bits<6> op, string instr_asm, SDNode OpNode,
                InstrItinClass itin> :
               TRI<op, (outs GPR:$dst), (ins GPR:$b, uimm16:$c),
                   !strconcat(instr_asm, " $dst, $b, $c"),
                   [(set GPR:$dst, (not (OpNode GPR:$b, immZExt16:$c)))], 
                   itin>;

// RI instruction with 16bit sign extended immediate.
class ArithRIs<bits<6> op, string instr_asm, SDNode OpNode,
               InstrItinClass itin> :
              TRI<op, (outs GPR:$dst), (ins GPR:$b, simm16:$c),
                  !strconcat(instr_asm, " $dst, $b, $c"),
                  [(set GPR:$dst, (OpNode GPR:$b, immSExt16:$c))], itin>;

// Fragment version of RI instruction with 16bit sign extended immediate.
class ArithFRIs<bits<6> op, string instr_asm, PatFrag OpNode,
                InstrItinClass itin> :
               TRI<op, (outs GPR:$dst), (ins GPR:$b, simm16:$c),
                   !strconcat(instr_asm, " $dst, $b, $c"),
                   [(set GPR:$dst, (OpNode GPR:$b, immSExt16:$c))], itin>;

// ArithRIs instruction with not in pattern.
class ArithNRIs<bits<6> op, string instr_asm, SDNode OpNode,
                InstrItinClass itin> :
               TRI<op, (outs GPR:$dst), (ins GPR:$b, simm16:$c),
                !strconcat(instr_asm, " $dst, $b, $c"),
                [(set GPR:$dst, (not (OpNode GPR:$b, immSExt16:$c)))], itin>;

// Shift instruction with 5bit zero extended immediate.
class ShiftI<bits<6> op, string instr_asm, SDNode OpNode,
                InstrItinClass itin> :
            TRIS<op, (outs GPR:$dst), (ins GPR:$b, uimm5:$c),
                 !strconcat(instr_asm, " $dst, $b, $c"),
                 [(set GPR:$dst, (OpNode GPR:$b, immZExt5:$c))], itin>;


//===----------------------------------------------------------------------===//
// Memory Access Instruction Formats
//===----------------------------------------------------------------------===//

class LoadM<bits<6> op, string instr_asm, PatFrag OpNode> :
           TRI<op, (outs GPR:$dst), (ins MEMri:$addr),
               !strconcat(instr_asm, " $dst, $addr"),
               [(set (i32 GPR:$dst), (OpNode ADDRri:$addr))], IILoad>;

class StoreM<bits<6> op, string instr_asm, PatFrag OpNode> :
            TRI<op, (outs), (ins GPR:$src, MEMri:$addr),
                !strconcat(instr_asm, " $addr, $src"),
                [(OpNode (i32 GPR:$src), ADDRri:$addr)], IIStore>;

//===----------------------------------------------------------------------===//
// Call (Branch and Link) Instructions and Patterns
//===----------------------------------------------------------------------===//

// FIXME: Note that Monarch adds registers to the end
// of the CALL operand list so they are known live into the call.
// Do we need to do this?
//
// SP is marked as a use to prevent stack-pointer assignments that
// appear immediately before calls from potentially appearing dead.
// ARM also marks FP as a use to prevent frame-pointer assignments
// from being moved above / below calls.
let isCall=1,
    // All calls clobber the link register and the non-callee-saved registers:
    Defs = [R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, RRA],
    Uses = [RSP] in {

  def CALLI : TI<0x3e, (outs), (ins calltarget:$target, variable_ops),
                 "calli    $target", [], IICall>;

  def CALL : TBR<0x36, (outs), (ins GPR:$dst, variable_ops),
                 "call     $dst", [(Mico32Call GPR:$dst)], IICall>;
}

// I think the linker should relax these as needed.
// FIXME:  check it really does.
def : Pat<(Mico32Call tglobaladdr:$addr), (CALLI tglobaladdr:$addr)>;
def : Pat<(Mico32Call texternalsym:$addr), (CALLI texternalsym:$addr)>;
def : Pat<(Mico32Call immSExt26ShftOff:$target),
            (CALLI (div4_xform immSExt26ShftOff:$target))>;


//===----------------------------------------------------------------------===//
// Branch and Return Instructions and Patterns
//===----------------------------------------------------------------------===//

// Treat this as call intinerary since it's always 4 cycles.
// FIXME: Is hasCtrlDep needed? Not sure what it does...
// Note the GPR can not be r30(ea) or R31(ba) aka ERET/BRET but since 
// the allocator is not allowed to use r30 or r31 we don't do anything special.
let isBranch = 1, isIndirectBranch=1,
    isTerminator=1, hasCtrlDep = 1, isBarrier = 1 in {
  def B : TBR<0x30, (outs), (ins GPR:$dst),
              "b       $dst",
              [(brind GPR:$dst)], IICall>;
}

// The is actually an alias for "b r29".
// processFunctionBeforeFrameFinalized() scans for uses of RA.
// If RA is found to be used in processFunctionBeforeFrameFinalized()
// emitPrologue/emitEpilogue will do save/restore of RA. If RA is marked
// used here it is always saved.
let isReturn = 1, isBarrier = 1, isTerminator = 1 /*, Uses = [RRA]*/ in {
  let ra = RRA.Num in
    def RET: TBR<0x30, (outs), (ins), 
		 "ret",
                 [(RetFlag)], IICall>;
}

// FIXME: Is hasCtrlDep needed? Not sure what it does...
let isBranch = 1, isTerminator=1, hasCtrlDep = 1, isBarrier = 1 in {

  def BI : TI<0x38, (outs), (ins brtarget:$target),
              "bi       $target",
              [(br bb:$target)], IICall>;

  // RI instruction with 16 bit sign extended immediate. The
  //FIXME: Need to account for shift?
  // 16 bit immediate is shifted left 2 bits to form a 16 bit word offset.
  // This is based on Mips.
  // Will this get matched if the branch becomes separated rfom the condition?
  let isPredicable = 1 in {
    class CBranch<bits<6> op, string instr_asm, PatFrag cond_op> : 
                  TRI<op, (outs), (ins GPR:$a, GPR:$b, brtarget:$offset),
                      !strconcat(instr_asm, " $a, $b, $offset"),
                      [(brcond (cond_op GPR:$a, GPR:$b), bb:$offset)],
                      IIBranch>;
  }
}

def BE     : CBranch<0x11, "be      ", seteq>;
def BG     : CBranch<0x12, "bg      ", setgt>;
def BGE    : CBranch<0x13, "bge     ", setge>;
def BGEU   : CBranch<0x14, "bgeu    ", setuge>;
def BGU    : CBranch<0x15, "bgu     ", setugt>;
def BNE    : CBranch<0x17, "bne     ", setne>;

// Derived from MBlaze:
// Reverse the sense of the test and the order of the operands.
def : Pat<(brcond (setlt  GPR:$L, GPR:$R), bb:$T),
          (BG GPR:$R, GPR:$L, bb:$T)>;
def : Pat<(brcond (setle GPR:$L, GPR:$R), bb:$T),
          (BGE GPR:$R, GPR:$L, bb:$T)>;
def : Pat<(brcond (setult  GPR:$L, GPR:$R), bb:$T),
          (BGU GPR:$R, GPR:$L, bb:$T)>;
def : Pat<(brcond (setule GPR:$L, GPR:$R), bb:$T),
          (BGEU GPR:$R, GPR:$L, bb:$T)>;

// brcond patterns - also from MIPS
def : Pat<(brcond GPR:$cond, bb:$dst),
          (BNE GPR:$cond, R0, bb:$dst)>;

def : Pat<(brcond (setne GPR:$lhs, 0), bb:$dst),
          (BNE GPR:$lhs, R0, bb:$dst)>;
def : Pat<(brcond (seteq GPR:$lhs, 0), bb:$dst),
          (BE GPR:$lhs, R0, bb:$dst)>;

//===----------------------------------------------------------------------===//
// Mico32 arithmetic instructions
//===----------------------------------------------------------------------===//

let isCommutable = 1, isAsCheapAsAMove = 1 in {
  def ADD    :  ArithRR<0x2d, "add     ", add,  IIAlu>;
  def AND    :  ArithRR<0x28, "and     ", and, IIAlu>;
  def OR     :  ArithRR<0x2e, "or      ", or, IIAlu>;
  def XOR    :  ArithRR<0x26, "xor     ", xor, IIAlu>;
}

let isAsCheapAsAMove = 1 in {
  //  NOT is a pseudo instruction for XNOR rX,rY,r0:
  let rb = R0.Num in  {
    def NOT    :  TRR<0x29, (outs GPR:$dst), (ins GPR:$a),
                      "not $dst, $a",
                      [(set GPR:$dst, (not GPR:$a))], IIAlu>;
  }

  def NOR    :  ArithNRR<0x21, "nor      ", or, IIAlu>;  // (not (or ...))
  def XNOR   :  ArithNRR<0x29, "xnor     ", xor, IIAlu>; // (not (xor ...))
}

let isAsCheapAsAMove = 1 in {
  def SUB    :  ArithRR<0x32, "sub     ", sub,  IIAlu>;
}

let isCommutable = 1, Predicates=[HasMUL] in {
  def MUL    : ArithRR<0x22, "mul     ", mul,   IIMul>;
}

let Predicates=[HasBarrel] in {
  def SRU    :  ArithRR<0x20, "sru     ", srl, IIShift>;
  def SR     :  ArithRR<0x25, "sr      ", sra, IIShift>;
  def SL     :  ArithRR<0x2f, "sl      ", shl, IIShift>;
  def SRUI   :  ShiftI<0x00, "srui    ", srl, IIShift>;
  def SRI    :  ShiftI<0x05, "sri     ", sra, IIShift>;
  def SLI    :  ShiftI<0x0f, "sli     ", shl, IIShift>;
}

let Predicates=[HasDIV] in {
// FIXME: check the scheduling on this.
  def DIVU   :  ArithRR<0x23, "divu  ", udiv, IIDiv>; 
  def MODU   :  ArithRR<0x31, "modu  ", urem, IIDiv>;
}

let Predicates=[HasSDIV] in {
//  The chip currently does not implement div or mod
// FIXME: check the scheduling on this.
//  def DIV   :  ArithRR<0x27, "div   ", sdiv, IFIXTHIS>;
// FIXME: check that mod does handle negative numbers the same as frem.
//  def MOD   :  ArithRR<0x35, "mod   ", srem, IFIXTHIS>;
}


//===----------------------------------------------------------------------===//
// Mico32 immediate mode arithmetic instructions
//===----------------------------------------------------------------------===//

let isAsCheapAsAMove = 1 in {
  def ORHI    :  TRI<0x1e, (outs GPR:$dst), (ins GPR:$b, i32imm:$c),
                     "orhi     $dst, $b, $c",
                     [(set GPR:$dst, (or GPR:$b, imm16ShiftedZExt:$c))], 
                     IIAlu>;
  def ORHIsym :  TRI<0x1e, (outs GPR:$dst), (ins GPR:$b, symbolHi:$c),
                     "orhi     $dst, $b, $c",
                     [(set GPR:$dst, (or GPR:$b, imm16ShiftedZExt:$c))], 
                     IIAlu>;

  def ANDI    :  ArithRIz<0x08, "andi    ", and, IIAlu>;
  def ORI     :  ArithRIz<0x0e, "ori     ", or, IIAlu>;
  def ORIsym  :  ArithRIsym<0x0e, "ori     ", or, IIAlu>;
  def XORI    :  ArithRIz<0x06, "xori    ", xor, IIAlu>;

  def NORI    :  ArithNRIz<0x01, "nori     ", or, IIAlu>;  // (not (ori ...))
  def XNORI   :  ArithNRIz<0x09, "xnori    ", xor, IIAlu>; // (not (xori ...))

  def ADDI    :  ArithRIs<0x0d, "addi    ", add, IIAlu>;
  def ADDIsym :  ArithRIsym<0x0d, "addi    ", add, IIAlu>;
}

let Predicates=[HasMUL] in {
  def MULI    :  ArithRIs<0x02, "muli    ", mul, IIMul>;
}

//===----------------------------------------------------------------------===//
// Mico32 memory access instructions
//===----------------------------------------------------------------------===//

let canFoldAsLoad = 1, isReMaterializable = 1 in {
  def LB      : LoadM<0x04, "lb      ", sextloadi8>;
  def LH      : LoadM<0x07, "lh      ", sextloadi16>;

  def LBU     : LoadM<0x10, "lbu     ", zextloadi8>;
  def LHU     : LoadM<0x0b, "lhu     ", zextloadi16>;
  def LW      : LoadM<0x0a, "lw      ", load>;
}

def SB      :  StoreM<0x0c, "sb      ", truncstorei8>;
def SH      :  StoreM<0x03, "sh      ", truncstorei16>;
def SW      :  StoreM<0x16, "sw      ", store>;

// Convert any extend loads into zero extend loads
def : Pat<(extloadi8  ADDRri:$addr), (i32 (LBU ADDRri:$addr))>;
def : Pat<(extloadi16 ADDRri:$addr), (i32 (LHU ADDRri:$addr))>;


//===----------------------------------------------------------------------===//
// Mico32 comparison instructions
//===----------------------------------------------------------------------===//
def CMPEI   : ArithFRIs<0x19, "cmpei   ", seteq, IICmp>;
def CMPGI   : ArithFRIs<0x1a, "cmpgi   ", setgt, IICmp>;
def CMPGEI  : ArithFRIs<0x1b, "cmpgei  ", setge, IICmp>;
def CMPGEUI : ArithFRIz<0x1c, "cmpgeui ", setuge, IICmp>;
def CMPGUI  : ArithFRIz<0x1d, "cmpgui  ", setugt, IICmp>;
def CMPNEI  : ArithFRIs<0x1f, "cmpnei  ", setne, IICmp>;

def CMPE    : ArithFRR<0x39, "cmpe    ", seteq, IICmp>;
def CMPG    : ArithFRR<0x3a, "cmpg    ", setgt, IICmp>;
def CMPGE   : ArithFRR<0x3b, "cmpge   ", setge, IICmp>;
def CMPGEU  : ArithFRR<0x3c, "cmpgeu  ", setuge, IICmp>;
def CMPGU   : ArithFRR<0x3d, "cmpgu   ", setugt, IICmp>;
def CMPNE   : ArithFRR<0x3f, "cmpne   ", setne, IICmp>;

// Reverse the sense of the test and the order of the operands.
def : Pat<(setlt  GPR:$L, GPR:$R), (CMPG  GPR:$R, GPR:$L)>;
def : Pat<(setle  GPR:$L, GPR:$R), (CMPGE   GPR:$R, GPR:$L)>;
def : Pat<(setult GPR:$L, GPR:$R), (CMPGU GPR:$R, GPR:$L)>;
def : Pat<(setule GPR:$L, GPR:$R), (CMPGEU  GPR:$R, GPR:$L)>;



//===----------------------------------------------------------------------===//
// Mico32 misc instructions
//===----------------------------------------------------------------------===//

def SEXTH : TSX<0x37, (outs GPR:$dst), (ins GPR:$b),
                "sexth   $dst, $b", 
                [(set GPR:$dst, (sext_inreg GPR:$b, i16))], IIAlu>;

def SEXTB : TSX<0x2c, (outs GPR:$dst), (ins GPR:$b),
                "sextb   $dst, $b", 
                [(set GPR:$dst, (sext_inreg GPR:$b, i8))], IIAlu>;

def SCALL : TSC<0x2b, 0x07, (outs), (ins), "scall   ", [], IICall>;
def BREAK : TSC<0x2b, 0x02, (outs), (ins), "break   ", [], IICall>;

def RCSR  : TRCSR<0x24, (outs GPR:$d), (ins CSR:$csr),
                  "rcsr    $d, $csr", [], IIAlu>;
def WCSR  : TCR<0x34, (outs CSR:$csr), (ins GPR:$a),
                "wcsr    $csr, $a", [], IIAlu>;


//===----------------------------------------------------------------------===//
// Misc. instructions
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Arbitrary patterns that map to one or more instructions
//===----------------------------------------------------------------------===//

// Small (16 bit) signed immediates.
def : Pat<(i32 immSExt16:$val),
          (ADDI R0, imm:$val)>;

// Small (16 bit) unsigned immediates.
def : Pat<(i32 immZExt16:$val),
          (ORI R0, imm:$val)>;

// Arbitrary immediate support.
def : Pat<(i32 imm:$val),
          (ORI (ORHI R0, (HI16 imm:$val)), (LO16 imm:$val))>;

// hi/lo relocs (from Mips and SPU)
// ISel custom lowering generates (Mico32Hi tglobaladdr:$in) and
// (Mico32Lo tglobaladdr:$in).  We further lower them here:
def : Pat<(Mico32Hi tglobaladdr:$in), (ORHIsym R0, tglobaladdr:$in)>;
def : Pat<(Mico32Lo tglobaladdr:$in), (ORIsym R0, tglobaladdr:$in)>;
def : Pat<(add GPR:$hi, (Mico32Lo tglobaladdr:$lo)),
          (ADDIsym GPR:$hi, tglobaladdr:$lo)>;

// ISel custom lowering generates (Mico32Hi tconstpool:$in) and
// (Mico32Lo tconstpool:$in).  We further lower them here:
def : Pat<(Mico32Hi tconstpool:$in), (ORHIsym R0, tconstpool:$in)>;
def : Pat<(Mico32Lo tconstpool:$in), (ORIsym R0, tconstpool:$in)>;
def : Pat<(add GPR:$hi, (Mico32Lo tconstpool:$lo)),
          (ADDIsym GPR:$hi, tconstpool:$lo)>;

// ISel custom lowering generates (Mico32Hi tjumptable:$in) and
// (Mico32Lo tjumptable:$in).  We further lower them here:
def : Pat<(Mico32Hi tjumptable:$in), (ORHIsym R0, tjumptable:$in)>;
def : Pat<(Mico32Lo tjumptable:$in), (ORIsym R0, tjumptable:$in)>;
def : Pat<(add GPR:$hi, (Mico32Lo tjumptable:$lo)),
          (ADDIsym GPR:$hi, tjumptable:$lo)>;

// A conditional move pattern.
// Cond is 0 or 1 on entry.
//   %tmp = sub 0, %cond   ; = (cond) ? 0xFFFFFFFF : 0
//   %tmpnot = not %tmp    ; 
//   %tval = and %tmp %trueval
//   %fval = and %tmpnot %falseval
//   %res = or %trueval %falseval
def : Pat<(select GPR:$which, GPR:$trueval, GPR:$falseval),
          (OR (AND GPR:$trueval, (SUB R0, GPR:$which)),
               (AND GPR:$falseval, (NOT (SUB R0, GPR:$which))))>;


