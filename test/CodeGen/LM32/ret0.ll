; RUN: llvm-as < %s | llc -march=lm32
; END.
; RUN: llvm-as < %s | llc -march=arm

define i32 @test() {
        ret i32 0
}
