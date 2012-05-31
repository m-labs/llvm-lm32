; RUN: llvm-as < %s | llc -march=mico32
; END.
; RUN: llvm-as < %s | llc

define i32 @main() {  
  ret i32 42
}
