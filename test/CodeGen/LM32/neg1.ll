; RUN: llvm-as < %s | llc -march=lm32
; END.
; Make sure this testcase codegens to the lda -1 instruction
; RUN: llvm-as < %s | llc -march=lm32 | grep {\\-1}

define i64 @bar() {
entry:
	ret i64 -1
}
