//===- Mico32InstrFormats.td - MB Instruction defs ---------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This is based on the MBlaze backend.

// Format specifies the encoding used by the instruction.  This is part of the
// ad-hoc solution used to emit machine instruction encodings by our machine
// code emitter.
class Format<bits<6> val> {
      bits<6> Value = val;
}

// Instruction Formats from Lattice manual and observed.
//
//       |31|30       26|25     21|20      16|15      11|10       5|4        0|
// RI    |0 | Op        | Reg A   | Reg D    | Immediate                      |
// RI5   |0 | Op        | Reg A   | Reg D    | 0                   |Immediate |
// RR    |1 | Op        | Reg A   | Reg B    | Reg D    | 0                   |
// RRac  |1 | Op        | Reg A   | 0        | Reg D    | 0                   |
// RRab  |1 | Op        | CSR     | Reg      | 0                              |
// RRa   |1 | Op        | Reg A   | 0                                         |
// I26   |1 |1|1|00/11|0| Immediate                                           |
// I5    |1 |01011      | 0                                        |subopcode |
//
def FPseudo : Format<0>;
def FRI     : Format<1>;  // ADD, OR, etc.
def FRI5    : Format<2>;  // SRI, SLI, SRUI
def FRR     : Format<3>;  // ADDI, ORI, etc.
def FRRac   : Format<4>;  // RCSR, SEXTB, SEXTH
def FRRab   : Format<5>;  // WCSR
def FRRa    : Format<6>;  // B, BRET, CALL, ERET (uses register as subopcode)
def FI5     : Format<7>;  // SCALL, BREAK
def FI26    : Format<8>;  // BI, CALLI

//===----------------------------------------------------------------------===//
//  Describe Mico32 instructions format
//
//  CPU INSTRUCTION FORMATS
//
//  opcode  - operation code.
//  rd      - dst reg.
//  ra      - first src. reg.
//  rb      - second src. reg.
//  imm5    - 5-bit immediate value. (RI5 format)
//  imm16   - 16-bit immediate value.
//  imm26   - 26-bit immediate value (FI26 format instructions).
//
//===----------------------------------------------------------------------===//

// Generic Mico32 Format
class Mico32Inst<bits<6> op, Format form, dag outs, dag ins, string asmstr,
                 list<dag> pattern, InstrItinClass itin> : Instruction {
  let Namespace = "Mico32";
  field bits<32> Inst;

  bits<6> opcode = op;
  Format Form = form;
  bits<6> FormBits = Form.Value;

  // Top 6 bits are the 'opcode' field
  let Inst{0-5} = opcode;

  // If the instruction is marked as a pseudo, set isCodeGenOnly so that the
  // assembler and disassmbler ignore it.
  let isCodeGenOnly = !eq(!cast<string>(form), "FPseudo");

  dag OutOperandList = outs;
  dag InOperandList  = ins;

  let AsmString   = asmstr;
  let Pattern     = pattern;
  let Itinerary   = itin;

  // TSFlags layout should be kept in sync with Mico32InstrInfo.h (MBlazeII 
  // namespace enum.
  // This is the bit-reversed opcode???
  let TSFlags{5-0}   = FormBits;
}

//===----------------------------------------------------------------------===//
// Pseudo instruction class
//===----------------------------------------------------------------------===//
class Mico32Pseudo<dag outs, dag ins, string asmstr, list<dag> pattern>:
      Mico32Inst<0x0, FPseudo, outs, ins, asmstr, pattern, IIPseudo>;

//===----------------------------------------------------------------------===//
// Type RR instruction class in Mico32 : <|opcode|ra|rb|rd|0|>
//===----------------------------------------------------------------------===//

class TRR<bits<6> op, bits<11> flags, dag outs, dag ins, string asmstr,
          list<dag> pattern, InstrItinClass itin> :
          Mico32Inst<op,FRRR,outs, ins, asmstr, pattern, itin>
{
  bits<5> ra;
  bits<5> rb;
  bits<5> rd;

  let Inst{21-25} = ra;
  let Inst{16-20} = rb;
  let Inst{11-15} = rd;
  let Inst{0-10} = 0;
}

//===----------------------------------------------------------------------===//
// Type RI instruction class in Mico32 : <|opcode|ra|rd|imm16|>
//===----------------------------------------------------------------------===//

class TRI<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern,
          InstrItinClass itin> :
          Mico32Inst<op, FRRI, outs, ins, asmstr, pattern, itin>
{
  bits<5>  ra;
  bits<5>  rd;
  bits<16> imm16;

  let Inst{21-25} = ra;
  let Inst{16-20} = rd;
  let Inst{0-15}  = imm16;
}

//===----------------------------------------------------------------------===//
// Type RI5 instruction class in Mico32 : <|opcode|ra|rd|0|imm5|>
//===----------------------------------------------------------------------===//

class TRI5<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern,
           InstrItinClass itin> :
           Mico32Inst<op, FRRI, outs, ins, asmstr, pattern, itin>
{
  bits<5>  ra;
  bits<5>  rd;
  bits<5> imm5;

  let Inst{21-25} = ra;
  let Inst{16-20} = rd;
  let Inst{5-15}  = 0;
  let Inst{0-4}   = imm5;
}


//===----------------------------------------------------------------------===//
// Type RRac instruction class in Mico32 : <|opcode|ra|0|rd|0|>
//===----------------------------------------------------------------------===//
class TRRac<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern,
            InstrItinClass itin> :
            Mico32Inst<op, FRI, outs, ins, asmstr, pattern, itin> {
  bits<5> rd;
  bits<5> ra;

  let Inst{21-25} = ra;
  let Inst{16-20} = 0;
  let Inst{11-15} = rd;
  let Inst{0-15} = 0;
}

//===----------------------------------------------------------------------===//
// Type RRab instruction class in Mico32 : <|opcode|cr|ra|0|>
//===----------------------------------------------------------------------===//
class TRRab<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern,
            InstrItinClass itin> :
            Mico32Inst<op, FRI, outs, ins, asmstr, pattern, itin> {
  bits<5> cr;
  bits<5> ra;

  let Inst{21-25} = cr;
  let Inst{16-20} = ra;
  let Inst{0-15} = 0;
}

//===----------------------------------------------------------------------===//
// Type RRa instruction class in Mico32 : <|opcode|ra|0|>
//===----------------------------------------------------------------------===//
class TRRa<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern,
           InstrItinClass itin> :
           Mico32Inst<op, FRI, outs, ins, asmstr, pattern, itin> {
  bits<5> ra;

  let Inst{21-25} = ra;
  let Inst{0-20} = 0;
}

//===----------------------------------------------------------------------===//
// Type I26 instruction class in Mico32 : <|opcode|imm26|>
//===----------------------------------------------------------------------===//
class TI26<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern,
           InstrItinClass itin> :
           Mico32Inst<op, FRI, outs, ins, asmstr, pattern, itin> {
  bits<26> imm26;

  let Inst{0-25} = imm26;
}

//===----------------------------------------------------------------------===//
// Type I5 instruction class in Mico32 : <|opcode|0|imm5|>
//===----------------------------------------------------------------------===//
class TI5<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern,
          InstrItinClass itin> :
          Mico32Inst<op, FRI, outs, ins, asmstr, pattern, itin> {
  bits<5> imm5;

  let Inst{5-25} = 0;
  let Inst{0-4} = imm5;
}

