; Make sure this testcase codegens to the lda -1 instruction
; RUN: llvm-as < %s | llc -march=mico32 | grep {\\-1}

define i64 @bar() {
entry:
	ret i64 -1
}
