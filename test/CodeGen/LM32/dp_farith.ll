; RUN: llvm-as -o - %s | llc -march=lm32
; END.
; RUN: llvm-as -o - %s | llc -march=cellspu > %t1.s
; RUN: grep dfa    %t1.s | count 2
; RUN: grep dfs    %t1.s | count 2
; RUN: grep dfm    %t1.s | count 6
; RUN: grep dfma   %t1.s | count 2
; RUN: grep dfms   %t1.s | count 2
; RUN: grep dfnms  %t1.s | count 4
;
; This file includes double precision floating point arithmetic instructions

define double @fadd(double %arg1, double %arg2) {
        %A = fadd double %arg1, %arg2
        ret double %A
}


define double @fsub(double %arg1, double %arg2) {
        %A = fsub double %arg1,  %arg2
        ret double %A
}

define double @fmul(double %arg1, double %arg2) {
        %A = fmul double %arg1,  %arg2
        ret double %A
}

define double @fma(double %arg1, double %arg2, double %arg3) {
        %A = fmul double %arg1,  %arg2
        %B = fadd double %A, %arg3
        ret double %B
}

define double @fms(double %arg1, double %arg2, double %arg3) {
        %A = fmul double %arg1,  %arg2
        %B = fsub double %A, %arg3
        ret double %B
}

; - (a * b - c)
define double @d_fnms_1(double %arg1, double %arg2, double %arg3) {
        %A = fmul double %arg1,  %arg2
        %B = fsub double %A, %arg3
        %C = fsub double -0.000000e+00, %B               ; <double> [#uses=1]
        ret double %C
}

; Annother way of getting fnms
; - ( a * b ) + c => c - (a * b)
define double @d_fnms_2(double %arg1, double %arg2, double %arg3) {
        %A = fmul double %arg1,  %arg2
        %B = fsub double %arg3, %A
        ret double %B
}

;define double @fdiv_1(double %arg1, double %arg2) {
;       %A = fdiv double %arg1,  %arg2  ; <double> [#uses=1]
;       ret double %A
;}
