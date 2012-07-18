//===- LM32RegisterInfo.td - The LM32 Register File --*- tablegen -*-=====//
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
//  Declarations that describe the LM32 register classes
//===----------------------------------------------------------------------===//

class LM32Reg<string n> : Register<n> {
  let Namespace = "LM32";
}


// M32GPR - One of the 32 32-bit general-purpose registers
class M32GPR<bits<5> num, string n> : LM32Reg<n> {
  field bits<5> Num = num;
}

// M32CSR - One of the 32-bit control and status registers.
class M32CSR<bits<5> num, string n> : LM32Reg<n> {
  field bits<5> Num = num;
}


//===----------------------------------------------------------------------===//
//  LM32 Registers
//===----------------------------------------------------------------------===//

// General-purpose registers
def R0  : M32GPR< 0, "r0">,  DwarfRegNum<[0]>;
def R1  : M32GPR< 1, "r1">,  DwarfRegNum<[1]>;
def R2  : M32GPR< 2, "r2">,  DwarfRegNum<[2]>;
def R3  : M32GPR< 3, "r3">,  DwarfRegNum<[3]>;
def R4  : M32GPR< 4, "r4">,  DwarfRegNum<[4]>;
def R5  : M32GPR< 5, "r5">,  DwarfRegNum<[5]>;
def R6  : M32GPR< 6, "r6">,  DwarfRegNum<[6]>;
def R7  : M32GPR< 7, "r7">,  DwarfRegNum<[7]>;
def R8  : M32GPR< 8, "r8">,  DwarfRegNum<[8]>;
def R9  : M32GPR< 9, "r9">,  DwarfRegNum<[9]>;
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
def RGP : M32GPR<26, "gp">,  DwarfRegNum<[26]>;
def RFP : M32GPR<27, "fp">,  DwarfRegNum<[27]>;
def RSP : M32GPR<28, "sp">,  DwarfRegNum<[28]>;
def RRA : M32GPR<29, "ra">,  DwarfRegNum<[29]>;
def REA : M32GPR<30, "ea">,  DwarfRegNum<[30]>;
def RBA : M32GPR<31, "ba">,  DwarfRegNum<[31]>;

// Control and Status Registers.
// FIXME: These dwarf numbers are made up.
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
def CSR : RegisterClass<"LM32", [i32], 32, (add
    IE, IM, IP, ICC, DCC, CC, CFG, EBA, CFG2)> {
  // None of the special purpose registers are allocatable.
  let isAllocatable = 0;
  //let CopyCost = -1;  // Don't allow copying of status registers.
}


// General Purpose Register Class.
// The order specified in the register list is implicitly defined to be the 
// register allocation order.
// Reserved registers are reserved by the function
// LM32RegisterInfo::getReservedRegs()
def GPR : RegisterClass<"LM32", [i32], 32, (add
    // Caller saved:
    R1, R2, R3, R4, R5, R6, R7, R8, R9, R10,
    // Callee saved:
    R11, R12, R13, R14, R15, R16, R17, R18, R19, R20,
    R21, R22, R23, R24, R25,
    RGP, // global pointer
    RFP, // frame pointer
    R0,  // always zero
    RSP, // stack pointer
    RRA, // return address
    REA, // return address for exceptions
    RBA  // return address for breakpoints
    )> ;
