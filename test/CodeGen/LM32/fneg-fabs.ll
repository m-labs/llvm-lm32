; RUN: llvm-as -o - %s | llc -march=lm32
; END.
; RUN: llvm-as -o - %s | llc -march=cellspu > %t1.s
; RUN: grep 32768   %t1.s | count 2
; RUN: grep xor     %t1.s | count 4
; RUN: grep and     %t1.s | count 2

define double @fneg_dp(double %X) {
        %Y = fsub double -0.000000e+00, %X
        ret double %Y
}

define float @fneg_sp(float %X) {
        %Y = fsub float -0.000000e+00, %X
        ret float %Y
}

declare double @fabs(double)

declare float @fabsf(float)

define double @fabs_dp(double %X) {
        %Y = call double @fabs( double %X )
        ret double %Y
}

define float @fabs_sp(float %X) {
        %Y = call float @fabsf( float %X )
        ret float %Y
}
