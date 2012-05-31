##===- lib/Target/Mico32/Makefile --------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

LEVEL = ../../..
LIBRARYNAME = LLVMMico32CodeGen
TARGET = Mico32

# Make sure that tblgen is run, first thing.
BUILT_SOURCES = Mico32GenRegisterInfo.inc Mico32GenInstrInfo.inc \
                Mico32GenAsmWriter.inc Mico32GenDAGISel.inc \
                Mico32GenCallingConv.inc Mico32GenSubtargetInfo.inc 

DIRS = TargetInfo MCTargetDesc

include $(LEVEL)/Makefile.common

