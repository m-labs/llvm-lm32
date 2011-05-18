set(LLVM_TARGET_DEFINITIONS Mico32.td)

tablegen(Mico32GenRegisterInfo.h.inc -gen-register-desc-header)
tablegen(Mico32GenRegisterNames.inc -gen-register-enums)
tablegen(Mico32GenRegisterInfo.inc -gen-register-desc)
tablegen(Mico32GenInstrNames.inc -gen-instr-enums)
tablegen(Mico32GenInstrInfo.inc -gen-instr-desc)
tablegen(Mico32GenCodeEmitter.inc -gen-emitter)
tablegen(Mico32GenAsmWriter.inc -gen-asm-writer)
tablegen(Mico32GenAsmMatcher.inc -gen-asm-matcher)
tablegen(Mico32GenDAGISel.inc -gen-dag-isel)
tablegen(Mico32GenCallingConv.inc -gen-callingconv)
tablegen(Mico32GenSubtarget.inc -gen-subtarget)
tablegen(Mico32GenIntrinsics.inc -gen-tgt-intrinsic)
tablegen(Mico32GenEDInfo.inc -gen-enhanced-disassembly-info)

add_llvm_target(Mico32CodeGen
  Mico32AsmPrinter.cpp
  Mico32MCAsmInfo.cpp
  Mico32ELFWriterInfo.cpp
  Mico32RegisterInfo.cpp
  Mico32FrameLowering.cpp
  Mico32SelectionDAGInfo.cpp
  Mico32ISelDAGToDAG.cpp
  Mico32Subtarget.cpp
  Mico32ISelLowering.cpp
  Mico32TargetMachine.cpp
  Mico32InstrInfo.cpp
  Mico32TargetObjectFile.cpp
  )

add_subdirectory(TargetInfo)

