//===- Mico32RegisterInfo.td - The Mico32 Register File --*- tablegen -*-=====//
// 
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// This TableGen file will generate a file that will "describe a class which 
// will store the register's number in the binary encoding of the instruction."
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the Mico32 register classes
//===----------------------------------------------------------------------===//

class Mico32Reg<string n> : Register<n> {
  let Namespace = "Mico32";
}


// M32GPR - One of the 32 32-bit general-purpose registers
class M32GPR<bits<5> num, string n> : Mico32Reg<n> {
  field bits<5> Num = num;
}

// M32CSR - One of the 32-bit control and status registers.
class M32CSR<bits<5> num, string n> : Mico32Reg<n> {
  field bits<5> Num = num;
}


//===----------------------------------------------------------------------===//
//  Mico32 Registers
//===----------------------------------------------------------------------===//

// General-purpose registers
def R0  : M32GPR< 0,  "r0">, DwarfRegNum<[0]>;
def R1  : M32GPR< 1,  "r1">, DwarfRegNum<[1]>;
def R2  : M32GPR< 2,  "r2">, DwarfRegNum<[2]>;
def R3  : M32GPR< 3,  "r3">, DwarfRegNum<[3]>;
def R4  : M32GPR< 4,  "r4">, DwarfRegNum<[4]>;
def R5  : M32GPR< 5,  "r5">, DwarfRegNum<[5]>;
def R6  : M32GPR< 6,  "r6">, DwarfRegNum<[6]>;
def R7  : M32GPR< 7,  "r7">, DwarfRegNum<[7]>;
def R8  : M32GPR< 8,  "r8">, DwarfRegNum<[8]>;
def R9  : M32GPR< 9,  "r9">, DwarfRegNum<[9]>;
def R10 : M32GPR<10, "r10">, DwarfRegNum<[10]>;
def R11 : M32GPR<11, "r11">, DwarfRegNum<[11]>;
def R12 : M32GPR<12, "r12">, DwarfRegNum<[12]>;
def R13 : M32GPR<13, "r13">, DwarfRegNum<[13]>;
def R14 : M32GPR<14, "r14">, DwarfRegNum<[14]>;
def R15 : M32GPR<15, "r15">, DwarfRegNum<[15]>;
def R16 : M32GPR<16, "r16">, DwarfRegNum<[16]>;
def R17 : M32GPR<17, "r17">, DwarfRegNum<[17]>;
def R18 : M32GPR<18, "r18">, DwarfRegNum<[18]>;
def R19 : M32GPR<19, "r19">, DwarfRegNum<[19]>;
def R20 : M32GPR<20, "r20">, DwarfRegNum<[20]>;
def R21 : M32GPR<21, "r21">, DwarfRegNum<[21]>;
def R22 : M32GPR<22, "r22">, DwarfRegNum<[22]>;
def R23 : M32GPR<23, "r23">, DwarfRegNum<[23]>;
def R24 : M32GPR<24, "r24">, DwarfRegNum<[24]>;
def R25 : M32GPR<25, "r25">, DwarfRegNum<[25]>;
def R26 : M32GPR<26, "r26">, DwarfRegNum<[26]>;
def R27 : M32GPR<27, "r27">, DwarfRegNum<[27]>;
def R28 : M32GPR<28, "r28">, DwarfRegNum<[28]>;
def R29 : M32GPR<29, "r29">, DwarfRegNum<[29]>;
def R30 : M32GPR<30, "r30">, DwarfRegNum<[30]>;
def R31 : M32GPR<31, "r31">, DwarfRegNum<[31]>;

// Control and Status Registers.
// FIXME: These dwarf numbera are made up.
def IE   : M32CSR<0, "ie">,    DwarfRegNum<[96]>;
def IM   : M32CSR<1, "im">,    DwarfRegNum<[97]>;
def IP   : M32CSR<2, "ip">,    DwarfRegNum<[98]>;
def ICC  : M32CSR<3, "icc">,   DwarfRegNum<[104]>;
def DCC  : M32CSR<4, "dcc">,   DwarfRegNum<[105]>;
def CC   : M32CSR<5, "cc">,    DwarfRegNum<[106]>;
def CFG  : M32CSR<6, "cfg">,   DwarfRegNum<[107]>;
def EBA  : M32CSR<7, "eba">,   DwarfRegNum<[108]>;
def CFG2 : M32CSR<10, "cfg2">, DwarfRegNum<[111]>;

//===----------------------------------------------------------------------===//
// Register Classes
//===----------------------------------------------------------------------===//

// Control and Status Register Class.
def CSR : RegisterClass<"Mico32", [i32], 32,
    [IE, IM, IP, ICC, DCC, CC, CFG, EBA, CFG2]> {
  let CopyCost = -1;  // Don't allow copying of status registers.
}


// General Purpose Register Class.
def GPR : RegisterClass<"Mico32", [i32], 32,
    // Caller saved:
    [R1, R2, R3, R4, R5, R6, R7, R8, R9, R10,
    // Callee saved:
    R11, R12, R13, R14, R15, R16, R17, R18, R19, R20,
    R21, R22, R23, R24, R25,
    // Unallocated:
    R26, // global pointer
    R27, // frame pointer
    R0,  // always zero
    R28, // stack pointer
    R29, // return address
    R30, // return address for exceptions
    R31  // return address for breakpoints
    ]>
{
//FIXME: do we need to reserve R26 (global pointer) and R27 (frame pointer)?
  let MethodProtos = [{
    iterator allocation_order_end(const MachineFunction &MF) const;
  }];
  let MethodBodies = [{
    GPRClass::iterator
    GPRClass::allocation_order_end(const MachineFunction &MF) const {
      // The last three registers on the list above are reserved
      return end()-7;
    }
  }];
}
