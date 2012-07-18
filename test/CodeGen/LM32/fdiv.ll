; RUN: llvm-as -o - %s | llc -march=lm32
; END.
; RUN: llvm-as -o - %s | llc -march=cellspu > %t1.s
; RUN: grep frest    %t1.s | count 2 
; RUN: grep -w fi    %t1.s | count 2 
; RUN: grep -w fm    %t1.s | count 2
; RUN: grep fma      %t1.s | count 2 
; RUN: grep fnms     %t1.s | count 4
; RUN: grep cgti     %t1.s | count 2
; RUN: grep selb     %t1.s | count 2
;
; This file includes standard floating point arithmetic instructions

define float @fdiv32(float %arg1, float %arg2) {
        %A = fdiv float %arg1,  %arg2
        ret float %A
}

define <2 x float> @fdiv_v2f32(<2 x float> %arg1, <2 x float> %arg2) {
        %A = fdiv <2 x float> %arg1,  %arg2
        ret <2 x float> %A
}
