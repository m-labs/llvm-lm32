; RUN: llvm-as -o - %s | llc -march=lm32
; END.
; RUN: llvm-as -o - %s | llc -march=cellspu -enable-unsafe-fp-math > %t1.s
; RUN: grep fa %t1.s | count 2
; RUN: grep fs %t1.s | count 2
; RUN: grep fm %t1.s | count 6
; RUN: grep fma %t1.s | count 2
; RUN: grep fms %t1.s | count 2
; RUN: grep fnms %t1.s | count 3
;
; This file includes standard floating point arithmetic instructions
; NOTE fdiv is tested separately since it is a compound operation

define float @fp_add(float %arg1, float %arg2) {
        %A = fadd float %arg1, %arg2     ; <float> [#uses=1]
        ret float %A
}

define float @fp_sub(float %arg1, float %arg2) {
        %A = fsub float %arg1,  %arg2    ; <float> [#uses=1]
        ret float %A
}

define float @fp_mul(float %arg1, float %arg2) {
        %A = fmul float %arg1,  %arg2    ; <float> [#uses=1]
        ret float %A
}

define float @fp_mul_add(float %arg1, float %arg2, float %arg3) {
        %A = fmul float %arg1,  %arg2    ; <float> [#uses=1]
        %B = fadd float %A, %arg3        ; <float> [#uses=1]
        ret float %B
}

define float @fp_mul_sub(float %arg1, float %arg2, float %arg3) {
        %A = fmul float %arg1,  %arg2    ; <float> [#uses=1]
        %B = fsub float %A, %arg3        ; <float> [#uses=1]
        ret float %B
}

; Test the straightforward way of getting fnms
; c - a * b
define float @fp_neg_mul_sub_1(float %arg1, float %arg2, float %arg3) {
        %A = fmul float %arg1,  %arg2
        %B = fsub float %arg3, %A
        ret float %B
}

; Test another way of getting fnms
; - ( a *b -c ) = c - a * b
define float @fp_neg_mul_sub_2(float %arg1, float %arg2, float %arg3) {
        %A = fmul float %arg1,  %arg2
        %B = fsub float %A, %arg3 
        %C = fsub float -0.0, %B
        ret float %C
}
