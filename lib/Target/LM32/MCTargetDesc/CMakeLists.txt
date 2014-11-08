add_llvm_library(LLVMLM32Desc
  LM32MCTargetDesc.cpp
  LM32MCAsmInfo.cpp
  )

add_dependencies(LLVMLM32Desc LM32CommonTableGen)

