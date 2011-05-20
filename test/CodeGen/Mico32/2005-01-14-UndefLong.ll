; RUN: llvm-as < %s | llc -march=mico32

define i64 @test() {
        ret i64 undef
}
