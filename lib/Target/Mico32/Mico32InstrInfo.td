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
// MBlaze type profiles
//===----------------------------------------------------------------------===//


//===----------------------------------------------------------------------===//
// MBlaze specific nodes
//===----------------------------------------------------------------------===//


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
def uimm16      : Operand<i32>;
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
class ArithRR<bits<6> op, string instr_asm, SDNode OpNode,
              InstrItinClass itin> :
             TRR<op, (outs GPR:$dst), (ins GPR:$b, GPR:$c),
                 !strconcat(instr_asm, "   $dst, $b, $c"),
                 [(set GPR:$dst, (OpNode GPR:$b, GPR:$c))], itin>;

class ArithNRR<bits<6> op, string instr_asm, SDNode OpNode,
              InstrItinClass itin> :
             TRR<op, (outs GPR:$dst), (ins GPR:$b, GPR:$c),
                 !strconcat(instr_asm, "   $dst, $b, $c"),
                 [(set GPR:$dst, (not (OpNode GPR:$b, GPR:$c)))], itin>;

class ArithNRIz<bits<6> op, string instr_asm, SDNode OpNode, list<dag> pattern,
                InstrItinClass itin> :
               TRI<op, (outs GPR:$dst), (ins GPR:$b, uimm16:$c),
                   !strconcat(instr_asm, "   $dst, $b, $c"),
                   [(set GPR:$dst, (not (OpNode GPR:$b, immZExt16:$c)))], 
                   itin>;

class ArithRIz<bits<6> op, string instr_asm, SDNode OpNode,
               InstrItinClass itin> :
              TRI<op, (outs GPR:$dst), (ins GPR:$b, uimm16:$c),
                  !strconcat(instr_asm, "   $dst, $b, $c"),
                  [(set GPR:$dst, (OpNode GPR:$b, immZExt16:$c))], itin>;

class ArithRIs<bits<6> op, string instr_asm, SDNode OpNode,
               InstrItinClass itin> :
              TRI<op, (outs GPR:$dst), (ins GPR:$b, simm16:$c),
                  !strconcat(instr_asm, "   $dst, $b, $c"),
                  [(set GPR:$dst, (OpNode GPR:$b, immSExt16:$c))], itin>;

class ShiftI<bits<6> op, string instr_asm, SDNode OpNode> :
            TRIS<op, (outs GPR:$dst), (ins GPR:$b, uimm5:$c),
                 !strconcat(instr_asm, "   $dst, $b, $c"),
                 [(set GPR:$dst, (OpNode GPR:$b, immZExt5:$c))], IIAlu>;


//===----------------------------------------------------------------------===//
// Misc Arithmetic Instruction Formats
//===----------------------------------------------------------------------===//


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
class Branch<bits<6> op, bits<5> br, bits<11> flags, string instr_asm> :
             TA<op, flags, (outs), (ins GPR:$target),
                !strconcat(instr_asm, "   $target"),
                [], IIBranch> {
  let rd = 0x0;
  let ra = br;
  let Form = FCCR;
}

class BranchI<bits<6> op, bits<5> br, string instr_asm> :
              TB<op, (outs), (ins brtarget:$target),
                 !strconcat(instr_asm, "   $target"),
                 [], IIBranch> {
  let rd = 0;
  let ra = br;
  let Form = FCCI;
}

//===----------------------------------------------------------------------===//
// Branch and Link Instruction Formats
//===----------------------------------------------------------------------===//
class BranchL<bits<6> op, bits<5> br, bits<11> flags, string instr_asm> :
              TA<op, flags, (outs), (ins GPR:$link, GPR:$target, variable_ops),
                 !strconcat(instr_asm, "   $link, $target"),
                 [], IIBranch> {
  let ra = br;
  let Form = FRCR;
}

class BranchLI<bits<6> op, bits<5> br, string instr_asm> :
               TB<op, (outs), (ins GPR:$link, calltarget:$target, variable_ops),
                  !strconcat(instr_asm, "   $link, $target"),
                  [], IIBranch> {
  let ra = br;
  let Form = FRCI;
}

//===----------------------------------------------------------------------===//
// Conditional Branch Instruction Formats
//===----------------------------------------------------------------------===//
class BranchC<bits<6> op, bits<5> br, bits<11> flags, string instr_asm> :
              TA<op, flags, (outs),
                 (ins GPR:$a, GPR:$b),
                 !strconcat(instr_asm, "   $a, $b"),
                 [], IIBranch> {
  let rd = br;
  let Form = FCRR;
}

class BranchCI<bits<6> op, bits<5> br, string instr_asm> :
               TB<op, (outs), (ins GPR:$a, brtarget:$offset),
                  !strconcat(instr_asm, "   $a, $offset"),
                  [], IIBranch> {
  let rd = br;
  let Form = FCRI;
}

//===----------------------------------------------------------------------===//
// Mico32 arithmetic instructions
//===----------------------------------------------------------------------===//

let isCommutable = 1, isAsCheapAsAMove = 1 in {
  def ADD    :  ArithRR<0x2d, "add   ", add,  IIAlu>;
  def AND    :  ArithRR<0x28, "and   ", and, IIAlu>;
  def OR     :  ArithRR<0x2e, "or    ", or, IIAlu>;
  def XOR    :  ArithRR<0x26, "xor   ", xor, IIAlu>;
  def NOR    :  ArithNRR<0x21, "nor    ", or, IIAlu>;
  def XNOR   :  ArithNRR<0x29, "xnor   ", xor, IIAlu>;
}

let isAsCheapAsAMove = 1 in {
  def SUB  :  ArithRR<0x32, "sub  ", sub,  IIAlu>;
}

let isCommutable = 1, Predicates=[HasMul] in {
  def MUL    : ArithRR<0x22, "mul    ", mul,   IIAlu>;
}

let Predicates=[HasBarrel] in {
  def SRU    :  ArithRR<0x20, "sru   ", srl, IIAlu>;
  def SR     :  ArithRR<0x25, "sr    ", sra, IIAlu>;
  def SL     :  ArithRR<0x2f, "sl    ", shl, IIAlu>;
  def SRUI   :  ShiftI<0x00, "srui  ", srl>;
  def SRI    :  ShiftI<0x05, "sri   ", sra>;
  def SLI    :  ShiftI<0x0f, "sli   ", shl>;
}

let Predicates=[HasDiv] in {
// FIXME: check the scheduling on this.
  def DIVU   :  ArithRR<0x23, "divu  ", udiv, IIAlu>; 
  def MODU   :  ArithRR<0x31, "modu  ", remu, IIAlu>;
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
  def ANDI    :  ArithRIz<0x08, "andi  ", and>;
  def ORI     :  ArithRIz<0x0e, "ori   ", or>;
  def XORI    :  ArithRIz<0x06, "xori  ", xor>;

  def NORI    :  ArithNRIz<0x01, "nori  ", or>;
  def XNORI   :  ArithNRIz<0x09, "xnori ", xor>;

  def ADDI    :  ArithRIs<0x0d, "addi  ", add>;
}

let Predicates=[HasMul] in {
  def MULI    :  ArithRIs<0x02, "muli  ", mul>;
}

//===----------------------------------------------------------------------===//
// Mico32 memory access instructions
//===----------------------------------------------------------------------===//

let canFoldAsLoad = 1, isReMaterializable = 1 in {
  def LB   : LoadM<0x04, "lb    ", sextloadi8>;
  def LH   : LoadM<0x07, "lh    ", sextloadi16>;

  def LBU  : LoadM<0x10, "lbu   ", zextloadi8>;
  def LHU  : LoadM<0x0b, "lhu   ", zextloadi16>;
  def LW   : LoadM<0x0a, "lw    ", load>;
}

def SB  :  StoreM<0x0c, "sb    ", truncstorei8>;
def SH  :  StoreM<0x03, "sh    ", truncstorei16>;
def SW  :  StoreM<0x16, "sw    ", store>;

//===----------------------------------------------------------------------===//
// Mico32 comparison instructions
//===----------------------------------------------------------------------===//
def CMPEI : CMPRI<0x19, "cmpei ", seteq>


//===----------------------------------------------------------------------===//
// MBlaze branch instructions
//===----------------------------------------------------------------------===//

let isBranch = 1, isTerminator = 1, hasCtrlDep = 1, isBarrier = 1 in {
  def BRI    :  BranchI<0x2E, 0x00, "bri    ">;
  def BRAI   :  BranchI<0x2E, 0x08, "brai   ">;
}

let isBranch = 1, isTerminator = 1, hasCtrlDep = 1 in {
  def BEQI   : BranchCI<0x2F, 0x00, "beqi   ">;
  def BNEI   : BranchCI<0x2F, 0x01, "bnei   ">;
  def BLTI   : BranchCI<0x2F, 0x02, "blti   ">;
  def BLEI   : BranchCI<0x2F, 0x03, "blei   ">;
  def BGTI   : BranchCI<0x2F, 0x04, "bgti   ">;
  def BGEI   : BranchCI<0x2F, 0x05, "bgei   ">;
}

let isBranch = 1, isIndirectBranch = 1, isTerminator = 1, hasCtrlDep = 1,
    isBarrier = 1 in {
  def BR     :   Branch<0x26, 0x00, 0x000, "br     ">;
  def BRA    :   Branch<0x26, 0x08, 0x000, "bra    ">;
}

let isBranch = 1, isIndirectBranch = 1, isTerminator = 1, hasCtrlDep = 1 in {
  def BEQ    :  BranchC<0x27, 0x00, 0x000, "beq    ">;
  def BNE    :  BranchC<0x27, 0x01, 0x000, "bne    ">;
  def BLT    :  BranchC<0x27, 0x02, 0x000, "blt    ">;
  def BLE    :  BranchC<0x27, 0x03, 0x000, "ble    ">;
  def BGT    :  BranchC<0x27, 0x04, 0x000, "bgt    ">;
  def BGE    :  BranchC<0x27, 0x05, 0x000, "bge    ">;
}

let isBranch = 1, isTerminator = 1, hasDelaySlot = 1, hasCtrlDep = 1,
    isBarrier = 1 in {
  def BRID   :  BranchI<0x2E, 0x10, "brid   ">;
  def BRAID  :  BranchI<0x2E, 0x18, "braid  ">;
}

let isBranch = 1, isTerminator = 1, hasDelaySlot = 1, hasCtrlDep = 1 in {
  def BEQID  : BranchCI<0x2F, 0x10, "beqid  ">;
  def BNEID  : BranchCI<0x2F, 0x11, "bneid  ">;
  def BLTID  : BranchCI<0x2F, 0x12, "bltid  ">;
  def BLEID  : BranchCI<0x2F, 0x13, "bleid  ">;
  def BGTID  : BranchCI<0x2F, 0x14, "bgtid  ">;
  def BGEID  : BranchCI<0x2F, 0x15, "bgeid  ">;
}

let isBranch = 1, isIndirectBranch = 1, isTerminator = 1,
    hasDelaySlot = 1, hasCtrlDep = 1, isBarrier = 1 in {
  def BRD    :   Branch<0x26, 0x10, 0x000, "brd    ">;
  def BRAD   :   Branch<0x26, 0x18, 0x000, "brad   ">;
}

let isBranch = 1, isIndirectBranch = 1, isTerminator = 1,
    hasDelaySlot = 1, hasCtrlDep = 1 in {
  def BEQD   :  BranchC<0x27, 0x10, 0x000, "beqd   ">;
  def BNED   :  BranchC<0x27, 0x11, 0x000, "bned   ">;
  def BLTD   :  BranchC<0x27, 0x12, 0x000, "bltd   ">;
  def BLED   :  BranchC<0x27, 0x13, 0x000, "bled   ">;
  def BGTD   :  BranchC<0x27, 0x14, 0x000, "bgtd   ">;
  def BGED   :  BranchC<0x27, 0x15, 0x000, "bged   ">;
}

let isCall =1, hasDelaySlot = 1,
    Defs = [R3,R4,R5,R6,R7,R8,R9,R10,R11,R12,CARRY],
    Uses = [R1] in {
  def BRLID  : BranchLI<0x2E, 0x14, "brlid  ">;
  def BRALID : BranchLI<0x2E, 0x1C, "bralid ">;
}

let isCall = 1, hasDelaySlot = 1,
    Defs = [R3,R4,R5,R6,R7,R8,R9,R10,R11,R12,CARRY],
    Uses = [R1] in {
  def BRLD   : BranchL<0x26, 0x14, 0x000, "brld   ">;
  def BRALD  : BranchL<0x26, 0x1C, 0x000, "brald  ">;
}

let isReturn=1, isTerminator=1, hasDelaySlot=1, isBarrier=1,
    rd=0x10, Form=FCRI in {
  def RTSD   : TB<0x2D, (outs), (ins GPR:$target, simm16:$imm),
                  "rtsd      $target, $imm",
                  [],
                  IIBranch>;
}

let isReturn=1, isTerminator=1, hasDelaySlot=1, isBarrier=1,
    rd=0x11, Form=FCRI in {
  def RTID   : TB<0x2D, (outs), (ins GPR:$target, simm16:$imm),
                  "rtid      $target, $imm",
                  [],
                  IIBranch>;
}

let isReturn=1, isTerminator=1, hasDelaySlot=1, isBarrier=1,
    rd=0x12, Form=FCRI in {
  def RTBD   : TB<0x2D, (outs), (ins GPR:$target, simm16:$imm),
                  "rtbd      $target, $imm",
                  [],
                  IIBranch>;
}

let isReturn=1, isTerminator=1, hasDelaySlot=1, isBarrier=1,
    rd=0x14, Form=FCRI in {
  def RTED   : TB<0x2D, (outs), (ins GPR:$target, simm16:$imm),
                  "rted      $target, $imm",
                  [],
                  IIBranch>;
}

//===----------------------------------------------------------------------===//
// Mico32 misc instructions
//===----------------------------------------------------------------------===//

def SEXTH : SX<0x37, (outs GPR:$dst), (ins GPR:$b),
               "sexth    $dst, $b", 
               [(set GPR:$dst, (sext_inreg GPR:$b, i16))], IIAlu>;

def SEXTB : SX<0x2c, (outs GPR:$dst), (ins GPR:$b),
               "sextb     $dst, $b", 
               [(set GPR:$dst, (sext_inreg GPR:$b, i8))], IIAlu>;

def SCALL : TSC<0x2b, 0x07, (outs), (ins), "scall ", [], IISC>;
def BREAK : TSC<0x2b, 0x02, (outs), (ins), "break ", [], IISC>;

def RCSR  : TRCSR<0x24, (outs GPR:$d), (ins CSR:$csr),
                  "rcsr  $d, $csr", [], IIAlu>;
def WCSR  : TCR<0x34, (outs CSR:$csr), (ins GPR:$a),
                "wcsr  $csr, $a", [], IIAlu>;


//===----------------------------------------------------------------------===//
// Misc. instructions
//===----------------------------------------------------------------------===//

let isCodeGenOnly=1 in {
  def ORI32   : LogicI32<0x28, "ori    ">;
  def BRLID32 : BranchLI<0x2E, 0x14, "brlid  ">;
}

let Form=FRCS in {
  def MFS : SPC<0x25, 0x2, (outs GPR:$dst), (ins SPR:$src),
                "mfs       $dst, $src", [], IIAlu>;
}

let Form=FCRCS in {
  def MTS : SPC<0x25, 0x3, (outs SPR:$dst), (ins GPR:$src),
                "mts       $dst, $src", [], IIAlu>;
}

def MSRSET : MSR<0x25, 0x20, (outs GPR:$dst), (ins uimm15:$set),
                 "msrset    $dst, $set", [], IIAlu>;

def MSRCLR : MSR<0x25, 0x22, (outs GPR:$dst), (ins uimm15:$clr),
                 "msrclr    $dst, $clr", [], IIAlu>;

let rd=0x0, Form=FCRR in {
  def WDC  : TA<0x24, 0x64, (outs), (ins GPR:$a, GPR:$b),
                "wdc       $a, $b", [], IIAlu>;
  def WDCF : TA<0x24, 0x74, (outs), (ins GPR:$a, GPR:$b),
                "wdc.flush $a, $b", [], IIAlu>;
  def WDCC : TA<0x24, 0x66, (outs), (ins GPR:$a, GPR:$b),
                "wdc.clear $a, $b", [], IIAlu>;
  def WIC  : TA<0x24, 0x68, (outs), (ins GPR:$a, GPR:$b),
                "wic       $a, $b", [], IIAlu>;
}

def BRK  :  BranchL<0x26, 0x0C, 0x000, "brk    ">;
def BRKI : BranchLI<0x2E, 0x0C, "brki   ">;

def IMM : MBlazeInst<0x2C, FCCI, (outs), (ins simm16:$imm),
                     "imm       $imm", [], IIAlu>;

//===----------------------------------------------------------------------===//
//  Arbitrary patterns that map to one or more instructions
//===----------------------------------------------------------------------===//

// Small immediates
def : Pat<(i32 0), (ADDK (i32 R0), (i32 R0))>;
def : Pat<(i32 immSExt16:$imm), (ADDIK (i32 R0), imm:$imm)>;
def : Pat<(i32 immZExt16:$imm), (ORI (i32 R0), imm:$imm)>;

// Arbitrary immediates
def : Pat<(i32 imm:$imm), (ADDIK (i32 R0), imm:$imm)>;

// In register sign extension
def : Pat<(sext_inreg GPR:$src, i16), (SEXT16 GPR:$src)>;
def : Pat<(sext_inreg GPR:$src, i8),  (SEXT8 GPR:$src)>;

// Call
def : Pat<(MBlazeJmpLink (i32 tglobaladdr:$dst)),
          (BRLID (i32 R15), tglobaladdr:$dst)>;

def : Pat<(MBlazeJmpLink (i32 texternalsym:$dst)),
          (BRLID (i32 R15), texternalsym:$dst)>;

def : Pat<(MBlazeJmpLink GPR:$dst),
          (BRALD (i32 R15), GPR:$dst)>;

// Shift Instructions
def : Pat<(shl GPR:$L, GPR:$R), (ShiftL GPR:$L, GPR:$R)>;
def : Pat<(sra GPR:$L, GPR:$R), (ShiftRA GPR:$L, GPR:$R)>;
def : Pat<(srl GPR:$L, GPR:$R), (ShiftRL GPR:$L, GPR:$R)>;

// SET_CC operations
def : Pat<(setcc (i32 GPR:$L), (i32 GPR:$R), SETEQ),
          (Select_CC (ADDIK (i32 R0), 1), (ADDIK (i32 R0), 0),
                     (CMP GPR:$R, GPR:$L), 1)>;
def : Pat<(setcc (i32 GPR:$L), (i32 GPR:$R), SETNE),
          (Select_CC (ADDIK (i32 R0), 1), (ADDIK (i32 R0), 0),
                     (CMP GPR:$R, GPR:$L), 2)>;
def : Pat<(setcc (i32 GPR:$L), (i32 GPR:$R), SETGT),
          (Select_CC (ADDIK (i32 R0), 1), (ADDIK (i32 R0), 0),
                     (CMP GPR:$R, GPR:$L), 3)>;
def : Pat<(setcc (i32 GPR:$L), (i32 GPR:$R), SETLT),
          (Select_CC (ADDIK (i32 R0), 1), (ADDIK (i32 R0), 0),
                     (CMP GPR:$R, GPR:$L), 4)>;
def : Pat<(setcc (i32 GPR:$L), (i32 GPR:$R), SETGE),
          (Select_CC (ADDIK (i32 R0), 1), (ADDIK (i32 R0), 0),
                     (CMP GPR:$R, GPR:$L), 5)>;
def : Pat<(setcc (i32 GPR:$L), (i32 GPR:$R), SETLE),
          (Select_CC (ADDIK (i32 R0), 1), (ADDIK (i32 R0), 0),
                     (CMP GPR:$R, GPR:$L), 6)>;
def : Pat<(setcc (i32 GPR:$L), (i32 GPR:$R), SETUGT),
          (Select_CC (ADDIK (i32 R0), 1), (ADDIK (i32 R0), 0),
                     (CMPU GPR:$R, GPR:$L), 3)>;
def : Pat<(setcc (i32 GPR:$L), (i32 GPR:$R), SETULT),
          (Select_CC (ADDIK (i32 R0), 1), (ADDIK (i32 R0), 0),
                     (CMPU GPR:$R, GPR:$L), 4)>;
def : Pat<(setcc (i32 GPR:$L), (i32 GPR:$R), SETUGE),
          (Select_CC (ADDIK (i32 R0), 1), (ADDIK (i32 R0), 0),
                     (CMPU GPR:$R, GPR:$L), 5)>;
def : Pat<(setcc (i32 GPR:$L), (i32 GPR:$R), SETULE),
          (Select_CC (ADDIK (i32 R0), 1), (ADDIK (i32 R0), 0),
                     (CMPU GPR:$R, GPR:$L), 6)>;

// SELECT operations
def : Pat<(select (i32 GPR:$C), (i32 GPR:$T), (i32 GPR:$F)),
          (Select_CC GPR:$T, GPR:$F, GPR:$C, 2)>;

// SELECT_CC
def : Pat<(selectcc (i32 GPR:$L), (i32 GPR:$R),
                    (i32 GPR:$T), (i32 GPR:$F), SETEQ),
          (Select_CC GPR:$T, GPR:$F, (CMP GPR:$R, GPR:$L), 1)>;
def : Pat<(selectcc (i32 GPR:$L), (i32 GPR:$R),
                    (i32 GPR:$T), (i32 GPR:$F), SETNE),
          (Select_CC GPR:$T, GPR:$F, (CMP GPR:$R, GPR:$L), 2)>;
def : Pat<(selectcc (i32 GPR:$L), (i32 GPR:$R),
                    (i32 GPR:$T), (i32 GPR:$F), SETGT),
          (Select_CC GPR:$T, GPR:$F, (CMP GPR:$R, GPR:$L), 3)>;
def : Pat<(selectcc (i32 GPR:$L), (i32 GPR:$R),
                    (i32 GPR:$T), (i32 GPR:$F), SETLT),
          (Select_CC GPR:$T, GPR:$F, (CMP GPR:$R, GPR:$L), 4)>;
def : Pat<(selectcc (i32 GPR:$L), (i32 GPR:$R),
                    (i32 GPR:$T), (i32 GPR:$F), SETGE),
          (Select_CC GPR:$T, GPR:$F, (CMP GPR:$R, GPR:$L), 5)>;
def : Pat<(selectcc (i32 GPR:$L), (i32 GPR:$R),
                    (i32 GPR:$T), (i32 GPR:$F), SETLE),
          (Select_CC GPR:$T, GPR:$F, (CMP GPR:$R, GPR:$L), 6)>;
def : Pat<(selectcc (i32 GPR:$L), (i32 GPR:$R),
                    (i32 GPR:$T), (i32 GPR:$F), SETUGT),
          (Select_CC GPR:$T, GPR:$F, (CMPU GPR:$R, GPR:$L), 3)>;
def : Pat<(selectcc (i32 GPR:$L), (i32 GPR:$R),
                    (i32 GPR:$T), (i32 GPR:$F), SETULT),
          (Select_CC GPR:$T, GPR:$F, (CMPU GPR:$R, GPR:$L), 4)>;
def : Pat<(selectcc (i32 GPR:$L), (i32 GPR:$R),
                    (i32 GPR:$T), (i32 GPR:$F), SETUGE),
          (Select_CC GPR:$T, GPR:$F, (CMPU GPR:$R, GPR:$L), 5)>;
def : Pat<(selectcc (i32 GPR:$L), (i32 GPR:$R),
                    (i32 GPR:$T), (i32 GPR:$F), SETULE),
          (Select_CC GPR:$T, GPR:$F, (CMPU GPR:$R, GPR:$L), 6)>;

// Ret instructions
def : Pat<(MBlazeRet GPR:$target), (RTSD GPR:$target, 0x8)>;
def : Pat<(MBlazeIRet GPR:$target), (RTID GPR:$target, 0x0)>;

// BR instructions
def : Pat<(br bb:$T), (BRID bb:$T)>;
def : Pat<(brind GPR:$T), (BRAD GPR:$T)>;

// BRCOND instructions
def : Pat<(brcond (setcc (i32 GPR:$L), (i32 GPR:$R), SETEQ), bb:$T),
          (BEQID (CMP GPR:$R, GPR:$L), bb:$T)>;
def : Pat<(brcond (setcc (i32 GPR:$L), (i32 GPR:$R), SETNE), bb:$T),
          (BNEID (CMP GPR:$R, GPR:$L), bb:$T)>;
def : Pat<(brcond (setcc (i32 GPR:$L), (i32 GPR:$R), SETGT), bb:$T),
          (BGTID (CMP GPR:$R, GPR:$L), bb:$T)>;
def : Pat<(brcond (setcc (i32 GPR:$L), (i32 GPR:$R), SETLT), bb:$T),
          (BLTID (CMP GPR:$R, GPR:$L), bb:$T)>;
def : Pat<(brcond (setcc (i32 GPR:$L), (i32 GPR:$R), SETGE), bb:$T),
          (BGEID (CMP GPR:$R, GPR:$L), bb:$T)>;
def : Pat<(brcond (setcc (i32 GPR:$L), (i32 GPR:$R), SETLE), bb:$T),
          (BLEID (CMP GPR:$R, GPR:$L), bb:$T)>;
def : Pat<(brcond (setcc (i32 GPR:$L), (i32 GPR:$R), SETUGT), bb:$T),
          (BGTID (CMPU GPR:$R, GPR:$L), bb:$T)>;
def : Pat<(brcond (setcc (i32 GPR:$L), (i32 GPR:$R), SETULT), bb:$T),
          (BLTID (CMPU GPR:$R, GPR:$L), bb:$T)>;
def : Pat<(brcond (setcc (i32 GPR:$L), (i32 GPR:$R), SETUGE), bb:$T),
          (BGEID (CMPU GPR:$R, GPR:$L), bb:$T)>;
def : Pat<(brcond (setcc (i32 GPR:$L), (i32 GPR:$R), SETULE), bb:$T),
          (BLEID (CMPU GPR:$R, GPR:$L), bb:$T)>;
def : Pat<(brcond (i32 GPR:$C), bb:$T),
          (BNEID GPR:$C, bb:$T)>;

// Jump tables, global addresses, and constant pools
def : Pat<(MBWrapper tglobaladdr:$in), (ORI (i32 R0), tglobaladdr:$in)>;
def : Pat<(MBWrapper tjumptable:$in),  (ORI (i32 R0), tjumptable:$in)>;
def : Pat<(MBWrapper tconstpool:$in),  (ORI (i32 R0), tconstpool:$in)>;

// Misc instructions
def : Pat<(and (i32 GPR:$lh), (not (i32 GPR:$rh))),(ANDN GPR:$lh, GPR:$rh)>;

// Arithmetic with immediates
def : Pat<(add (i32 GPR:$in), imm:$imm),(ADDIK GPR:$in, imm:$imm)>;
def : Pat<(or (i32 GPR:$in), imm:$imm),(ORI GPR:$in, imm:$imm)>;
def : Pat<(xor (i32 GPR:$in), imm:$imm),(XORI GPR:$in, imm:$imm)>;

// Convert any extend loads into zero extend loads
def : Pat<(i32 (extloadi8 ADDRri:$src)), (LBU ADDRri:$src)>;
def : Pat<(i32 (extloadi16 ADDRri:$src)), (LHU ADDRri:$src)>;

// zextload bool,extload bool -> zextload byte (from SPARC)
//def : Pat<(i32 (extloadi1 ADDRri:$src)), (LBU ADDRri:$src)>;
//def : Pat<(i32 (zextloadi1 ADDRri:$src)), (LBU ADDRri:$src)>;

// Peepholes
def : Pat<(store (i32 0), iaddr:$dst), (SWI (i32 R0), iaddr:$dst)>;
