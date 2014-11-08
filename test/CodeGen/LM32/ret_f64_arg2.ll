; RUN: llvm-as < %s | llc -march=lm32

define double @test_f64(double %a1, double %a2) {
        ret double %a2
}

