; RUN: llc -march=lm32 %s
; END.
; RUN: llvm-as < %s | llc -march=arm | not grep {add.*#0}

define i32 @foo() {
entry:
;	%A = alloca [1123 x i32], align 16		; <[1123 x i32]*> [#uses=1]
;	%B = alloca [3123 x i32], align 16		; <[3123 x i32]*> [#uses=1]
;	%C = alloca [12312 x i32], align 16		; <[12312 x i32]*> [#uses=1]
	%A = alloca [1123 x i32], align 4		; <[1123 x i32]*> [#uses=1]
	%B = alloca [3123 x i32], align 4		; <[3123 x i32]*> [#uses=1]
	%C = alloca [6312 x i32], align 4		; <[6312 x i32]*> [#uses=1]
	%tmp = call i32 (...)* @bar( [3123 x i32]* %B, [1123 x i32]* %A, [6312 x i32]* %C )		; <i32> [#uses=0]
	ret i32 undef
}

declare i32 @bar(...)
