; RUN: llvm-as < %s | llc -march=lm32

define i32 @test(i32 %a1) {
        ret i32 %a1
}
