//===- Mico32InstrInfo.td - MBlaze Instruction defs --------*- tablegen -*-===//
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


//===----------------------------------------------------------------------===//
// Mico32 specific nodes
//===----------------------------------------------------------------------===//

def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_Mico32CallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;

def callseq_end   : SDNode<"ISD::CALLSEQ_END", SDT_Mico32CallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;



//===----------------------------------------------------------------------===//
// Mico32 Instruction Predicate Definitions.
//===----------------------------------------------------------------------===//
def HasBarrel    : Predicate<"Subtarget.hasBarrel()">;
def NoBarrel     : Predicate<"!Subtarget.hasBarrel()">;
def HasDiv       : Predicate<"Subtarget.hasDiv()">;
def HasMul       : Predicate<"Subtarget.hasMul()">;
def HasSignX     : Predicate<"Subtarget.hasSignX()">;
def HasUserIns   : Predicate<"Subtarget.hasUserIns()">;
def HasFP        : Predicate<"Subtarget.hasFP()">;

//===----------------------------------------------------------------------===//
// Mico32 Operand, Complex Patterns and Transformations Definitions.
//===----------------------------------------------------------------------===//

// This is from MBlaze - need to see how it works.
def Mico32MemAsmOperand : AsmOperandClass {
  let Name = "Mem";
  let SuperClasses = [];
}

// Instruction operand types
def brtarget    : Operand<OtherVT>;
def calltarget  : Operand<i32>;
def simm16      : Operand<i32>;
def uimm5       : Operand<i32>;

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


// from PPC / X86
// immSExt16 predicate - True if the immediate fits in a 16-bit sign extended
// field.  Used by instructions like 'addi'.
def immSExt16  : PatLeaf<(imm), [{
  if (N->getValueType(0) == MVT::i32)
    return (int32_t)N->getZExtValue() == (int16_t)N->getZExtValue();
  else if (N->getValueType(0) == MVT::i64)
    return (int64_t)N->getZExtValue() == (int16_t)N->getZExtValue();
  else {
    assert(0);
  }
}]>;

// immZExt16 predicate - True if the immediate fits in a 16-bit zero extended
// field.  Used by instructions like 'ori'.
def immZExt16  : PatLeaf<(imm), [{
  return (uint64_t)N->getZExtValue() == (uint16_t)N->getZExtValue();
}], LO16>;


// From PowerPC: 
// imm16ShiftedZExt predicate - True if only bits in the top 16-bits of the
// immediate are set.  Used by instructions like 'oris'.
def imm16ShiftedZExt : PatLeaf<(imm), [{
  return (N->getZExtValue() & ~uint64_t(0xFFFF0000)) == 0;
}], HI16>;


// shift amount field must fit in 5 bits.
def immZExt5 : PatLeaf<(imm), [{
  return N->getZExtValue() == ((N->getZExtValue()) & 0x1f) ;
}]>;

// Addressing modes.
// Note SelectAddrRegImm is defined in MonarchISelDAGToDAG.cpp.
// See the maillist thread "Using frameindex in a pattern",
// "ComplexPattern in child ISel nodes",
// SelectAddrRegImm is defined in Mico32ISelDAGToDAG.cpp
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
//  The following I think is not quite right:
// If the node is of the type [frameindex] and "SelectAddrRegImm" returns true
// then the pattern is matched.  SelectAddrRegImm returns the number of operands
// specified, 2 in this case, these 2 operands are then passed as the arguments
// to the node using the ADDRri pattern.
//
// Mico32 Address Mode. SDNode frameindex could possibily be a match
// since load and store instructions from stack used it.
// From MBlaze:
def ADDRri : ComplexPattern<i32, 2, "SelectAddrRegImm", [frameindex], []>;
def ADDRrr : ComplexPattern<i32, 2, "SelectAddrRegReg", [], []>;

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
let Defs = [R28], Uses = [R28] in {
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
                 !strconcat(instr_asm, "   $dst, $b, $c"),
                 [(set GPR:$dst, (OpNode GPR:$b, GPR:$c))], itin>;

// Fragmant version of RR instruction
class ArithFRR<bits<6> op, string instr_asm, PatFrag OpNode,
               InstrItinClass itin> :
              TRR<op, (outs GPR:$dst), (ins GPR:$b, GPR:$c),
                  !strconcat(instr_asm, "   $dst, $b, $c"),
                  [(set GPR:$dst, (OpNode GPR:$b, GPR:$c))], itin>;

// RR instruction with not in pattern.
class ArithNRR<bits<6> op, string instr_asm, SDNode OpNode,
              InstrItinClass itin> :
             TRR<op, (outs GPR:$dst), (ins GPR:$b, GPR:$c),
                 !strconcat(instr_asm, "   $dst, $b, $c"),
                 [(set GPR:$dst, (not (OpNode GPR:$b, GPR:$c)))], itin>;

// Fragment version of RR instruction with not in pattern.
class ArithFNRR<bits<6> op, string instr_asm, PatFrag OpNode,
               InstrItinClass itin> :
              TRR<op, (outs GPR:$dst), (ins GPR:$b, GPR:$c),
                  !strconcat(instr_asm, "   $dst, $b, $c"),
                  [(set GPR:$dst, (not (OpNode GPR:$b, GPR:$c)))], itin>;

// RI instruction with 16bit zero extended immediate.
class ArithRIz<bits<6> op, string instr_asm, SDNode OpNode,
               InstrItinClass itin> :
              TRI<op, (outs GPR:$dst), (ins GPR:$b, uimm16:$c),
                  !strconcat(instr_asm, "   $dst, $b, $c"),
                  [(set GPR:$dst, (OpNode GPR:$b, immZExt16:$c))], itin>;

// Fragment version of RI instruction with 16bit zero extended immediate.
class ArithFRIz<bits<6> op, string instr_asm, PatFrag OpNode,
                InstrItinClass itin> :
               TRI<op, (outs GPR:$dst), (ins GPR:$b, uimm16:$c),
                   !strconcat(instr_asm, "   $dst, $b, $c"),
                   [(set GPR:$dst, (OpNode GPR:$b, immZExt16:$c))], itin>;

// ArithRIz with not in pattern.
class ArithNRIz<bits<6> op, string instr_asm, SDNode OpNode,
                InstrItinClass itin> :
               TRI<op, (outs GPR:$dst), (ins GPR:$b, uimm16:$c),
                   !strconcat(instr_asm, "   $dst, $b, $c"),
                   [(set GPR:$dst, (not (OpNode GPR:$b, immZExt16:$c)))], 
                   itin>;

// Fragment version of ArithRIz with not in pattern.
class ArithFNRIz<bits<6> op, string instr_asm, PatFrag OpNode,
                 InstrItinClass itin> :
                TRI<op, (outs GPR:$dst), (ins GPR:$b, uimm16:$c),
                    !strconcat(instr_asm, "   $dst, $b, $c"),
                    [(set GPR:$dst, (not (OpNode GPR:$b, immZExt16:$c)))], 
                    itin>;

// RI instruction with 16bit sign extended immediate.
class ArithRIs<bits<6> op, string instr_asm, SDNode OpNode,
               InstrItinClass itin> :
              TRI<op, (outs GPR:$dst), (ins GPR:$b, simm16:$c),
                  !strconcat(instr_asm, "   $dst, $b, $c"),
                  [(set GPR:$dst, (OpNode GPR:$b, immSExt16:$c))], itin>;

// Fragment version of RI instruction with 16bit sign extended immediate.
class ArithFRIs<bits<6> op, string instr_asm, PatFrag OpNode,
                InstrItinClass itin> :
               TRI<op, (outs GPR:$dst), (ins GPR:$b, simm16:$c),
                   !strconcat(instr_asm, "   $dst, $b, $c"),
                   [(set GPR:$dst, (OpNode GPR:$b, immSExt16:$c))], itin>;

// ArithRIs instruction with not in pattern.
class ArithNRIs<bits<6> op, string instr_asm, SDNode OpNode,
                InstrItinClass itin> :
               TRI<op, (outs GPR:$dst), (ins GPR:$b, simm16:$c),
                !strconcat(instr_asm, "   $dst, $b, $c"),
                [(set GPR:$dst, (not (OpNode GPR:$b, immSExt16:$c)))], itin>;

// Shift instruction with 5bit zero extended immediate.
class ShiftI<bits<6> op, string instr_asm, SDNode OpNode,
                InstrItinClass itin> :
            TRIS<op, (outs GPR:$dst), (ins GPR:$b, uimm5:$c),
                 !strconcat(instr_asm, "   $dst, $b, $c"),
                 [(set GPR:$dst, (OpNode GPR:$b, immZExt5:$c))], itin>;


//===----------------------------------------------------------------------===//
// Memory Access Instruction Formats
//===----------------------------------------------------------------------===//

class LoadM<bits<6> op, string instr_asm, PatFrag OpNode> :
           TRI<op, (outs GPR:$dst), (ins MEMri:$addr),
               !strconcat(instr_asm, "   $dst, $addr"),
               [(set (i32 GPR:$dst), (OpNode ADDRri:$addr))], IILoad>;

class StoreM<bits<6> op, string instr_asm, PatFrag OpNode> :
            TRI<op, (outs), (ins GPR:$dst, MEMri:$addr),
                !strconcat(instr_asm, "   $dst, $addr"),
                [(OpNode (i32 GPR:$dst), ADDRri:$addr)], IIStore>;

//===----------------------------------------------------------------------===//
// Branch Instruction Formats
//===----------------------------------------------------------------------===//
//class Branch<bits<6> op, bits<5> br, bits<11> flags, string instr_asm> :
//             TA<op, flags, (outs), (ins GPR:$target),
//                !strconcat(instr_asm, "   $target"),
//                [], IIBranch> {
//  let rd = 0x0;
//  let ra = br;
//  let Form = FCCR;
//}
//
//class BranchI<bits<6> op, bits<5> br, string instr_asm> :
//              TB<op, (outs), (ins brtarget:$target),
//                 !strconcat(instr_asm, "   $target"),
//                 [], IIBranch> {
//  let rd = 0;
//  let ra = br;
//  let Form = FCCI;
//}
//
////===----------------------------------------------------------------------===//
//// Branch and Link Instruction Formats
////===----------------------------------------------------------------------===//
//class BranchL<bits<6> op, bits<5> br, bits<11> flags, string instr_asm> :
//              TA<op, flags, (outs), (ins GPR:$link, GPR:$target, variable_ops),
//                 !strconcat(instr_asm, "   $link, $target"),
//                 [], IIBranch> {
//  let ra = br;
//  let Form = FRCR;
//}
//
//class BranchLI<bits<6> op, bits<5> br, string instr_asm> :
//               TB<op, (outs), (ins GPR:$link, calltarget:$target, variable_ops),
//                  !strconcat(instr_asm, "   $link, $target"),
//                  [], IIBranch> {
//  let ra = br;
//  let Form = FRCI;
//}
//
////===----------------------------------------------------------------------===//
//// Conditional Branch Instruction Formats
////===----------------------------------------------------------------------===//
//class BranchC<bits<6> op, bits<5> br, bits<11> flags, string instr_asm> :
//              TA<op, flags, (outs),
//                 (ins GPR:$a, GPR:$b),
//                 !strconcat(instr_asm, "   $a, $b"),
//                 [], IIBranch> {
//  let rd = br;
//  let Form = FCRR;
//}
//
//class BranchCI<bits<6> op, bits<5> br, string instr_asm> :
//               TB<op, (outs), (ins GPR:$a, brtarget:$offset),
//                  !strconcat(instr_asm, "   $a, $offset"),
//                  [], IIBranch> {
//  let rd = br;
//  let Form = FCRI;
//}
//

//===----------------------------------------------------------------------===//
// Mico32 arithmetic instructions
//===----------------------------------------------------------------------===//

let isCommutable = 1, isAsCheapAsAMove = 1 in {
  def ADD    :  ArithRR<0x2d, "add     ", add,  IIAlu>;
  def AND    :  ArithRR<0x28, "and     ", and, IIAlu>;
  def OR     :  ArithRR<0x2e, "or      ", or, IIAlu>;
  def XOR    :  ArithRR<0x26, "xor     ", xor, IIAlu>;
  def NOR    :  ArithNRR<0x21, "nor      ", or, IIAlu>;  // (not (or ...))
  def XNOR   :  ArithNRR<0x29, "xnor     ", xor, IIAlu>; // (not (xor ...))
}

let isAsCheapAsAMove = 1 in {
  def SUB    :  ArithRR<0x32, "sub     ", sub,  IIAlu>;
}

let isCommutable = 1, Predicates=[HasMul] in {
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

let Predicates=[HasDiv] in {
// FIXME: check the scheduling on this.
  def DIVU   :  ArithRR<0x23, "divu  ", udiv, IIDiv>; 
  def MODU   :  ArithRR<0x31, "modu  ", urem, IIDiv>;
}

let Predicates=[HasFP] in {
//  The chip currently does not implement div or mod
// FIXME: check the scheduling on this.
//  def DIV   :  ArithRR<0x27, "div   ", sdiv, IFIXTHIS>;
// FIXME: check that mod does handle negative numbers the same as frem.
//  def MOD   :  ArithRR<0x35, "mod   ", frem, IFIXTHIS>;
}


//===----------------------------------------------------------------------===//
// Mico32 immediate mode arithmetic instructions
//===----------------------------------------------------------------------===//

let isAsCheapAsAMove = 1 in {
  def ANDI    :  ArithRIz<0x08, "andi    ", and, IIAlu>;
  def ORI     :  ArithRIz<0x0e, "ori     ", or, IIAlu>;
  def XORI    :  ArithRIz<0x06, "xori    ", xor, IIAlu>;

  def NORI    :  ArithNRIz<0x01, "nori     ", or, IIAlu>;  // (not (ori ...))
  def XNORI   :  ArithNRIz<0x09, "xnori    ", xor, IIAlu>; // (not (xori ...))

  def ADDI    :  ArithRIs<0x0d, "addi    ", add, IIAlu>;
}

let Predicates=[HasMul] in {
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

//===----------------------------------------------------------------------===//
// Mico32 comparison instructions
//===----------------------------------------------------------------------===//
def CMPEI   : ArithFRIs<0x19, "cmpei   ", seteq, IICmp>;
def CMPGI   : ArithFRIs<0x1a, "cmpgi   ", setgt, IICmp>;
def CMPGEI  : ArithFRIs<0x1b, "cmpgei  ", setge, IICmp>;
def CMPGEUI : ArithFRIz<0x1c, "cmpgeui ", setuge, IICmp>;
def CMPGUI  : ArithFRIz<0x1d, "cmpgui  ", setugt, IICmp>;
def CMPNEI  : ArithFNRIz<0x1f, "cmpnei  ", seteq, IICmp>;  // (not (seteq ...))

def CMPE    : ArithFRR<0x39, "cmpe    ", seteq, IICmp>;
def CMPG    : ArithFRR<0x3a, "cmpg    ", setgt, IICmp>;
def CMPGE   : ArithFRR<0x3b, "cmpge   ", setge, IICmp>;
def CMPGEU  : ArithFRR<0x3c, "cmpgeu  ", setuge, IICmp>;
def CMPGU   : ArithFRR<0x3d, "cmpgu   ", setugt, IICmp>;
def CMPNE   : ArithFNRR<0x1f, "cmpne   ", seteq, IICmp>;  // (not (seteq ...))

//===----------------------------------------------------------------------===//
// MBlaze branch instructions
//===----------------------------------------------------------------------===//

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

