; mico32 supports returning <4 x i32> at most.
; nand_pass.ll has supported versions of this test.
; XFAIL: *
; RUN: llvm-as -o - %s | llc -march=mico32
; END.
; RUN: llvm-as -o - %s | llc -march=cellspu > %t1.s
; RUN: grep nand   %t1.s | count 90
; RUN: grep and    %t1.s | count 94
; RUN: grep xsbh   %t1.s | count 2
; RUN: grep xshw   %t1.s | count 4
target datalayout = "E-p:32:32:128-f64:64:128-f32:32:128-i64:32:128-i32:32:128-i16:16:128-i8:8:128-i1:8:128-a0:0:128-v128:128:128-s0:128:128"
target triple = "spu"

define <4 x i32> @nand_v4i32_1(<4 x i32> %arg1, <4 x i32> %arg2) {
        %A = and <4 x i32> %arg2, %arg1      ; <<4 x i32>> [#uses=1]
        %B = xor <4 x i32> %A, < i32 -1, i32 -1, i32 -1, i32 -1 >
        ret <4 x i32> %B
}

define <4 x i32> @nand_v4i32_2(<4 x i32> %arg1, <4 x i32> %arg2) {
        %A = and <4 x i32> %arg1, %arg2      ; <<4 x i32>> [#uses=1]
        %B = xor <4 x i32> %A, < i32 -1, i32 -1, i32 -1, i32 -1 >
        ret <4 x i32> %B
}

define <8 x i16> @nand_v8i16_1(<8 x i16> %arg1, <8 x i16> %arg2) {
        %A = and <8 x i16> %arg2, %arg1      ; <<8 x i16>> [#uses=1]
        %B = xor <8 x i16> %A, < i16 -1, i16 -1, i16 -1, i16 -1,
                                 i16 -1, i16 -1, i16 -1, i16 -1 >
        ret <8 x i16> %B
}

define <8 x i16> @nand_v8i16_2(<8 x i16> %arg1, <8 x i16> %arg2) {
        %A = and <8 x i16> %arg1, %arg2      ; <<8 x i16>> [#uses=1]
        %B = xor <8 x i16> %A, < i16 -1, i16 -1, i16 -1, i16 -1,
                                 i16 -1, i16 -1, i16 -1, i16 -1 >
        ret <8 x i16> %B
}

define <16 x i8> @nand_v16i8_1(<16 x i8> %arg1, <16 x i8> %arg2) {
        %A = and <16 x i8> %arg2, %arg1      ; <<16 x i8>> [#uses=1]
        %B = xor <16 x i8> %A, < i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1,
                                    i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1,
                                    i8 -1, i8 -1, i8 -1, i8 -1 >
        ret <16 x i8> %B
}

define <16 x i8> @nand_v16i8_2(<16 x i8> %arg1, <16 x i8> %arg2) {
        %A = and <16 x i8> %arg1, %arg2      ; <<16 x i8>> [#uses=1]
        %B = xor <16 x i8> %A, < i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1,
                                    i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1,
                                    i8 -1, i8 -1, i8 -1, i8 -1 >
        ret <16 x i8> %B
}
