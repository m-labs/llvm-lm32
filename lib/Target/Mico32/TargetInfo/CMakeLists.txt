include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. 
                     ${CMAKE_CURRENT_SOURCE_DIR}/.. )

add_llvm_library(LLVMMico32Info
  Mico32TargetInfo.cpp
  )

add_dependencies(LLVMMico32Info Mico32CommonTableGen)
