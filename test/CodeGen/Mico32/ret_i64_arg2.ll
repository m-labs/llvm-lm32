; RUN: llvm-as < %s | llc -march=mico32

define i64 @test_i64(i64 %a1, i64 %a2) {
        ret i64 %a2
}

