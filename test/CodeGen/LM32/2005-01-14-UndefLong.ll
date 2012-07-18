; RUN: llvm-as < %s | llc -march=lm32

define i64 @test() {
        ret i64 undef
}
