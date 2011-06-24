; RUN: llvm-as < %s | llc -march=mico32 | grep __adddf3

; Dead argument should reserve an FP register.
define double @bar(double %DEAD, double %X, double %Y) {
        %tmp.2 = fadd double %X, %Y              ; <double> [#uses=1]
        ret double %tmp.2
}
