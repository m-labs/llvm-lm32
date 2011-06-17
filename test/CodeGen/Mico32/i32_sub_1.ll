; Make sure this testcase codegens to the ctpop instruction
; RUN: llvm-as < %s | llc -march=mico32


define signext i32 @foo(i32 signext %x) {
entry:
	%tmp.1 = add i32 %x, -1		; <int> [#uses=1]
	ret i32 %tmp.1
}
