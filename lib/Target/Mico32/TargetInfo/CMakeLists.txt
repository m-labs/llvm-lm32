include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. 
                     ${CMAKE_CURRENT_SOURCE_DIR}/.. )

add_llvm_library(LLVMMico32Info
  Mico32TargetInfo.cpp
  )

add_llvm_library_dependencies(LLVMMico32Info
  LLVMMC
  LLVMSupport
  LLVMTarget
  )

add_dependencies(LLVMMico32Info Mico32CommonTableGen)
