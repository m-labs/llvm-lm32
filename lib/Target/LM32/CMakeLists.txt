set(LLVM_TARGET_DEFINITIONS Mico32.td)

tablegen(LLVM Mico32GenRegisterInfo.inc -gen-register-info)
tablegen(LLVM Mico32GenInstrInfo.inc -gen-instr-info)
tablegen(LLVM Mico32GenCodeEmitter.inc -gen-emitter)
tablegen(LLVM Mico32GenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM Mico32GenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM Mico32GenDAGISel.inc -gen-dag-isel)
tablegen(LLVM Mico32GenCallingConv.inc -gen-callingconv)
tablegen(LLVM Mico32GenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM Mico32GenIntrinsics.inc -gen-tgt-intrinsic)
tablegen(LLVM Mico32GenEDInfo.inc -gen-enhanced-disassembly-info)
add_public_tablegen_target(Mico32CommonTableGen)

add_llvm_target(Mico32CodeGen
  Mico32AsmPrinter.cpp
  Mico32ELFWriterInfo.cpp
  Mico32FrameLowering.cpp
  Mico32InstrInfo.cpp
  Mico32ISelDAGToDAG.cpp
  Mico32ISelLowering.cpp
  Mico32RegisterInfo.cpp
  Mico32SelectionDAGInfo.cpp
  Mico32Subtarget.cpp
  Mico32TargetMachine.cpp
  Mico32TargetObjectFile.cpp
  )

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)

