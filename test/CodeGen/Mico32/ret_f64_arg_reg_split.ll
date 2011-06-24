; RUN: llvm-as < %s | llc -march=mico32

define double @test_double_arg_reg_split(i32 %a1, double %a2) {
        ret double %a2
}

