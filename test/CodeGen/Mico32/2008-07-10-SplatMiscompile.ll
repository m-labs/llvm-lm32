; mico32 only supports returning <2 x i32> at most.
; RUN: llvm-as < %s | llc -march=mico32
; END.
; RUN: llvm-as < %s | llc -march=ppc32 -mcpu=g5 | grep vadduhm
; RUN: llvm-as < %s | llc -march=ppc32 -mcpu=g5 | grep vsubuhm

define <2 x i32> @test() nounwind {
	ret <2 x i32> < i32 4293066722, i32 4293066722>
}

define <2 x i32> @test2() nounwind {
	ret <2 x i32> < i32 1114129, i32 1114129>
}
