; RUN: llvm-as < %s | llc -march=lm32
; END.
; RUN: llvm-as < %s | llc -march=arm -mattr=+v6 | \
; RUN:   grep sxtb | count 1
; RUN: llvm-as < %s | llc -march=arm -mattr=+v6 | \
; RUN:   grep sxtab | count 1

define signext i8 @test1(i32 %A) {
	%B = lshr i32 %A, 8
	%C = shl i32 %A, 24
	%D = or i32 %B, %C
	%E = trunc i32 %D to i8
	ret i8 %E
}

define signext i32 @test2(i32 %A, i32 %X) {
	%B = lshr i32 %A, 8
	%C = shl i32 %A, 24
	%D = or i32 %B, %C
	%E = trunc i32 %D to i8
        %F = sext i8 %E to i32
        %G = add i32 %F, %X
	ret i32 %G
}
