; Test to make sure that the 'private' is used correctly.
;
; RUN: llc -march=lm32 < %s > %t
; RUN: grep \\\.Lfoo: %t
; RUN: grep calli.*\\\.Lfoo %t
; RUN: grep \\\.Lbaz: %t
; END.
; RUN: grep lw.*\\\$baz %t

define private void @foo() {
        ret void
}

@baz = private global i32 4

define i32 @bar() {
        call void @foo()
	%1 = load i32* @baz, align 4
        ret i32 %1
}
