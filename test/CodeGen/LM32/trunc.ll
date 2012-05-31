; lm32 supports returning <2 x i32> at most.
; trunc_pass.ll has supported versions of this test.
; XFAIL: *
; RUN: llvm-as -o - %s | llc -march=lm32
; END.
; RUN: llvm-as -o - %s | llc -march=cellspu > %t1.s
; RUN: grep shufb   %t1.s | count 19
; RUN: grep {ilhu.*1799}  %t1.s | count 1
; RUN: grep {ilhu.*771}  %t1.s | count 2
; RUN: grep {ilhu.*1543}  %t1.s | count 1
; RUN: grep {ilhu.*1029}  %t1.s | count 1
; RUN: grep {ilhu.*515}  %t1.s | count 1
; RUN: grep {ilhu.*3855}  %t1.s | count 1
; RUN: grep {ilhu.*3599}  %t1.s | count 1
; RUN: grep {ilhu.*3085}  %t1.s | count 1
; RUN: grep {iohl.*3855}  %t1.s | count 1
; RUN: grep {iohl.*3599}  %t1.s | count 2
; RUN: grep {iohl.*1543}  %t1.s | count 2
; RUN: grep {iohl.*771}  %t1.s | count 2
; RUN: grep {iohl.*515}  %t1.s | count 1
; RUN: grep {iohl.*1799}  %t1.s | count 1
; RUN: grep lqa  %t1.s | count 1
; RUN: grep cbd  %t1.s | count 4
; RUN: grep chd  %t1.s | count 3
; RUN: grep cwd  %t1.s | count 1
; RUN: grep cdd  %t1.s | count 1

; ModuleID = 'trunc.bc'

define <16 x i8> @trunc_i128_i8(i128 %u, <16 x i8> %v) {
entry:
	%0 = trunc i128 %u to i8
    %tmp1 = insertelement <16 x i8> %v, i8 %0, i32 15 
    ret <16 x i8> %tmp1
}

define <8 x i16> @trunc_i128_i16(i128 %u, <8 x i16> %v) {
entry:
    %0 = trunc i128 %u to i16
    %tmp1 = insertelement <8 x i16> %v, i16 %0, i32 8 
    ret <8 x i16> %tmp1
}

define <4 x i32> @trunc_i128_i32(i128 %u, <4 x i32> %v) {
entry:
    %0 = trunc i128 %u to i32
    %tmp1 = insertelement <4 x i32> %v, i32 %0, i32 2
    ret <4 x i32> %tmp1
}

define <2 x i64> @trunc_i128_i64(i128 %u, <2 x i64> %v) {
entry:
    %0 = trunc i128 %u to i64
    %tmp1 = insertelement <2 x i64> %v, i64 %0, i32 1
    ret <2 x i64> %tmp1
}

define <16 x i8> @trunc_i64_i8(i64 %u, <16 x i8> %v) {
entry:
    %0 = trunc i64 %u to i8
    %tmp1 = insertelement <16 x i8> %v, i8 %0, i32 10
    ret <16 x i8> %tmp1
}

define <8 x i16> @trunc_i64_i16(i64 %u, <8 x i16> %v) {
entry:
    %0 = trunc i64 %u to i16
    %tmp1 = insertelement <8 x i16> %v, i16 %0, i32 6
    ret <8 x i16> %tmp1
}

define i32 @trunc_i64_i32(i64 %u) {
entry:
    %0 = trunc i64 %u to i32
    ret i32 %0
}

define <16 x i8> @trunc_i32_i8(i32 %u, <16 x i8> %v) {
entry:
    %0 = trunc i32 %u to i8
    %tmp1 = insertelement <16 x i8> %v, i8 %0, i32 7
    ret <16 x i8> %tmp1
}

define <8 x i16> @trunc_i32_i16(i32 %u, <8 x i16> %v) {
entry:
    %0 = trunc i32 %u to i16
    %tmp1 = insertelement <8 x i16> %v, i16 %0, i32 3
    ret <8 x i16> %tmp1
}

define <16 x i8> @trunc_i16_i8(i16 %u, <16 x i8> %v) {
entry:
    %0 = trunc i16 %u to i8
    %tmp1 = insertelement <16 x i8> %v, i8 %0, i32 5
    ret <16 x i8> %tmp1
}
