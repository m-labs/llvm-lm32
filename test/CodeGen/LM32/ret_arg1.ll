; RUN: llvm-as < %s | llc -march=mico32

define i32 @test(i32 %a1) {
        ret i32 %a1
}
