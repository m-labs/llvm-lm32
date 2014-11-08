; lm32 only supports returning <2 x i32> at most.
; XFAIL: *
; RUN: llvm-as < %s | llc -march=lm32
; END.
; RUN: llvm-as < %s | llc -march=ppc32 -mcpu=g5 | grep sth

define <8 x i16> @insert(<8 x i16> %foo, i16 %a) nounwind  {
entry:
	%vecext = insertelement <8 x i16> %foo, i16 %a, i32 7		; <i8> [#uses=1]
	ret <8 x i16> %vecext
}

