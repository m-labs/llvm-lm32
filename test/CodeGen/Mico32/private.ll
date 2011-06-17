; Test to make sure that the 'private' is used correctly.
;
; RUN: llc < %s -march=mico32 > %t
; RUN: grep \\\$foo: %t
; RUN: grep calli.*\\\$foo %t
; RUN: grep \\\$baz: %t
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
