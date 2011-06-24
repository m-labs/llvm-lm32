; mico32 supports retuning <2 x i32> at most.
; or_ops_pass.ll has supported versions of this test.
; XFAIL: *
; RUN: llvm-as -o - %s | llc -march=mico32
; END.
; RUN: llvm-as -o - %s | llc -march=cellspu > %t1.s
; RUN: grep and    %t1.s | count 2
; RUN: grep orc    %t1.s | count 85
; RUN: grep ori    %t1.s | count 30
; RUN: grep orhi   %t1.s | count 30
; RUN: grep orbi   %t1.s | count 15
target datalayout = "E-p:32:32:128-f64:64:128-f32:32:128-i64:32:128-i32:32:128-i16:16:128-i8:8:128-i1:8:128-a0:0:128-v128:128:128-s0:128:128"
target triple = "spu"

; OR instruction generation:
define <4 x i32> @or_v4i32_1(<4 x i32> %arg1, <4 x i32> %arg2) {
        %A = or <4 x i32> %arg1, %arg2
        ret <4 x i32> %A
}

define <4 x i32> @or_v4i32_2(<4 x i32> %arg1, <4 x i32> %arg2) {
        %A = or <4 x i32> %arg2, %arg1
        ret <4 x i32> %A
}

define <8 x i16> @or_v8i16_1(<8 x i16> %arg1, <8 x i16> %arg2) {
        %A = or <8 x i16> %arg1, %arg2
        ret <8 x i16> %A
}

define <8 x i16> @or_v8i16_2(<8 x i16> %arg1, <8 x i16> %arg2) {
        %A = or <8 x i16> %arg2, %arg1
        ret <8 x i16> %A
}

define <16 x i8> @or_v16i8_1(<16 x i8> %arg1, <16 x i8> %arg2) {
        %A = or <16 x i8> %arg2, %arg1
        ret <16 x i8> %A
}

define <16 x i8> @or_v16i8_2(<16 x i8> %arg1, <16 x i8> %arg2) {
        %A = or <16 x i8> %arg1, %arg2
        ret <16 x i8> %A
}

; ORC instruction generation:
define <4 x i32> @orc_v4i32_1(<4 x i32> %arg1, <4 x i32> %arg2) {
        %A = xor <4 x i32> %arg2, < i32 -1, i32 -1, i32 -1, i32 -1 >
        %B = or <4 x i32> %arg1, %A
        ret <4 x i32> %B
}

define <4 x i32> @orc_v4i32_2(<4 x i32> %arg1, <4 x i32> %arg2) {
        %A = xor <4 x i32> %arg1, < i32 -1, i32 -1, i32 -1, i32 -1 >
        %B = or <4 x i32> %arg2, %A
        ret <4 x i32> %B
}

define <4 x i32> @orc_v4i32_3(<4 x i32> %arg1, <4 x i32> %arg2) {
        %A = xor <4 x i32> %arg1, < i32 -1, i32 -1, i32 -1, i32 -1 >
        %B = or <4 x i32> %A, %arg2
        ret <4 x i32> %B
}

define <8 x i16> @orc_v8i16_1(<8 x i16> %arg1, <8 x i16> %arg2) {
        %A = xor <8 x i16> %arg2, < i16 -1, i16 -1, i16 -1, i16 -1,
                                    i16 -1, i16 -1, i16 -1, i16 -1 >
        %B = or <8 x i16> %arg1, %A
        ret <8 x i16> %B
}

define <8 x i16> @orc_v8i16_2(<8 x i16> %arg1, <8 x i16> %arg2) {
        %A = xor <8 x i16> %arg1, < i16 -1, i16 -1, i16 -1, i16 -1,
                                    i16 -1, i16 -1, i16 -1, i16 -1 >
        %B = or <8 x i16> %arg2, %A
        ret <8 x i16> %B
}

define <16 x i8> @orc_v16i8_1(<16 x i8> %arg1, <16 x i8> %arg2) {
        %A = xor <16 x i8> %arg1, < i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1,
                                    i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1,
                                    i8 -1, i8 -1, i8 -1, i8 -1 >
        %B = or <16 x i8> %arg2, %A
        ret <16 x i8> %B
}

define <16 x i8> @orc_v16i8_2(<16 x i8> %arg1, <16 x i8> %arg2) {
        %A = xor <16 x i8> %arg2, < i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1,
                                    i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1,
                                    i8 -1, i8 -1, i8 -1, i8 -1 >
        %B = or <16 x i8> %arg1, %A
        ret <16 x i8> %B
}

define <16 x i8> @orc_v16i8_3(<16 x i8> %arg1, <16 x i8> %arg2) {
        %A = xor <16 x i8> %arg2, < i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1,
                                    i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1,
                                    i8 -1, i8 -1, i8 -1, i8 -1 >
        %B = or <16 x i8> %A, %arg1
        ret <16 x i8> %B
}

; ORI instruction generation (i32 data type):
define <4 x i32> @ori_v4i32_1(<4 x i32> %in) {
        %tmp2 = or <4 x i32> %in, < i32 511, i32 511, i32 511, i32 511 >
        ret <4 x i32> %tmp2
}

define <4 x i32> @ori_v4i32_2(<4 x i32> %in) {
        %tmp2 = or <4 x i32> %in, < i32 510, i32 510, i32 510, i32 510 >
        ret <4 x i32> %tmp2
}

define <4 x i32> @ori_v4i32_3(<4 x i32> %in) {
        %tmp2 = or <4 x i32> %in, < i32 -1, i32 -1, i32 -1, i32 -1 >
        ret <4 x i32> %tmp2
}

define <4 x i32> @ori_v4i32_4(<4 x i32> %in) {
        %tmp2 = or <4 x i32> %in, < i32 -512, i32 -512, i32 -512, i32 -512 >
        ret <4 x i32> %tmp2
}

; ORHI instruction generation (i16 data type):
define <8 x i16> @orhi_v8i16_1(<8 x i16> %in) {
        %tmp2 = or <8 x i16> %in, < i16 511, i16 511, i16 511, i16 511,
                                    i16 511, i16 511, i16 511, i16 511 >
        ret <8 x i16> %tmp2
}

define <8 x i16> @orhi_v8i16_2(<8 x i16> %in) {
        %tmp2 = or <8 x i16> %in, < i16 510, i16 510, i16 510, i16 510,
                                    i16 510, i16 510, i16 510, i16 510 >
        ret <8 x i16> %tmp2
}

define <8 x i16> @orhi_v8i16_3(<8 x i16> %in) {
        %tmp2 = or <8 x i16> %in, < i16 -1, i16 -1, i16 -1, i16 -1, i16 -1,
                                    i16 -1, i16 -1, i16 -1 >
        ret <8 x i16> %tmp2
}

define <8 x i16> @orhi_v8i16_4(<8 x i16> %in) {
        %tmp2 = or <8 x i16> %in, < i16 -512, i16 -512, i16 -512, i16 -512,
                                    i16 -512, i16 -512, i16 -512, i16 -512 >
        ret <8 x i16> %tmp2
}

; ORBI instruction generation (i8 data type):
define <16 x i8> @orbi_v16i8(<16 x i8> %in) {
        %tmp2 = or <16 x i8> %in, < i8 42, i8 42, i8 42, i8 42, i8 42, i8 42,
                                    i8 42, i8 42, i8 42, i8 42, i8 42, i8 42,
                                    i8 42, i8 42, i8 42, i8 42 >
        ret <16 x i8> %tmp2
}
