add_llvm_library(LLVMMico32Desc
  Mico32MCTargetDesc.cpp
  Mico32MCAsmInfo.cpp
  )

add_dependencies(LLVMMico32Desc Mico32CommonTableGen)

