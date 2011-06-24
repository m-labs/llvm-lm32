; RUN: llvm-as < %s | llc -march=mico32
; END.
; RUN: llvm-as < %s | llc -march=ppc32 -mcpu=g5 -o %t -f
; RUN: grep vrlw %t
; RUN: not grep spr %t
; RUN: not grep vrsave %t

define <2 x i32> @test_rol() {
        ret <2 x i32> < i32 -11534337, i32 -11534337 >
}

define <2 x i32> @test_arg(<2 x i32> %A, <2 x i32> %B) {
        %C = add <2 x i32> %A, %B               ; <<2 x i32>> [#uses=1]
        ret <2 x i32> %C
}

