set(LLVM_TARGET_DEFINITIONS Mico32.td)

tablegen(Mico32GenRegisterInfo.inc -gen-register-info)
tablegen(Mico32GenInstrInfo.inc -gen-instr-info)
tablegen(Mico32GenCodeEmitter.inc -gen-emitter)
tablegen(Mico32GenAsmWriter.inc -gen-asm-writer)
tablegen(Mico32GenAsmMatcher.inc -gen-asm-matcher)
tablegen(Mico32GenDAGISel.inc -gen-dag-isel)
tablegen(Mico32GenCallingConv.inc -gen-callingconv)
tablegen(Mico32GenSubtarget.inc -gen-subtarget)
tablegen(Mico32GenIntrinsics.inc -gen-tgt-intrinsic)
tablegen(Mico32GenEDInfo.inc -gen-enhanced-disassembly-info)
add_public_tablegen_target(Mico32CommonTableGen)

add_llvm_target(Mico32CodeGen
  Mico32AsmPrinter.cpp
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

add_llvm_library_dependencies(LLVMMico32CodeGen
  LLVMAsmPrinter
  LLVMCodeGen
  LLVMCore
  LLVMMico32AsmPrinter
  LLVMMico32Desc
  LLVMMico32Info
  LLVMMC
  LLVMSelectionDAG
  LLVMSupport
  LLVMTarget
  )

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)

