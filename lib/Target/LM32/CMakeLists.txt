set(LLVM_TARGET_DEFINITIONS LM32.td)

tablegen(LLVM LM32GenRegisterInfo.inc -gen-register-info)
tablegen(LLVM LM32GenInstrInfo.inc -gen-instr-info)
tablegen(LLVM LM32GenCodeEmitter.inc -gen-emitter)
tablegen(LLVM LM32GenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM LM32GenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM LM32GenDAGISel.inc -gen-dag-isel)
tablegen(LLVM LM32GenCallingConv.inc -gen-callingconv)
tablegen(LLVM LM32GenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM LM32GenIntrinsics.inc -gen-tgt-intrinsic)
tablegen(LLVM LM32GenEDInfo.inc -gen-enhanced-disassembly-info)
add_public_tablegen_target(LM32CommonTableGen)

add_llvm_target(LM32CodeGen
  LM32AsmPrinter.cpp
  LM32ELFWriterInfo.cpp
  LM32FrameLowering.cpp
  LM32InstrInfo.cpp
  LM32ISelDAGToDAG.cpp
  LM32ISelLowering.cpp
  LM32RegisterInfo.cpp
  LM32SelectionDAGInfo.cpp
  LM32Subtarget.cpp
  LM32TargetMachine.cpp
  LM32TargetObjectFile.cpp
  )

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)

