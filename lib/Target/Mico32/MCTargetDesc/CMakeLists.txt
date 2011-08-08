add_llvm_library(LLVMMico32Desc
  Mico32MCTargetDesc.cpp
  Mico32MCAsmInfo.cpp
  )

add_llvm_library_dependencies(LLVMMico32Desc
  LLVMMC
  LLVMMico32Info
  LLVMSupport
  )

add_dependencies(LLVMMico32Desc Mico32CommonTableGen)

