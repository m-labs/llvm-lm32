; RUN: llvm-as < %s | llc -march=mico32

define i32 @test(i32 %a1, i32 %a2) {
        ret i32 %a2
}

