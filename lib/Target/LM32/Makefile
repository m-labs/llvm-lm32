##===- lib/Target/LM32/Makefile ----------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

LEVEL = ../../..
LIBRARYNAME = LLVMLM32CodeGen
TARGET = LM32

# Make sure that tblgen is run, first thing.
BUILT_SOURCES = LM32GenRegisterInfo.inc LM32GenInstrInfo.inc \
                LM32GenAsmWriter.inc LM32GenDAGISel.inc \
                LM32GenCallingConv.inc LM32GenSubtargetInfo.inc 

DIRS = TargetInfo MCTargetDesc

include $(LEVEL)/Makefile.common

