; Test to make sure that the 'private' is used correctly.
;
; RUN: llc < %s -march=lm32 | FileCheck %s
; CHECK: .Lfoo:
; CHECK-LABEL: bar:
; CHECK: calli .Lfoo
; CHECK: .Lbaz:
; CHECK: .long 4

define private void @foo() {
        ret void
}

@baz = private global i32 4

define i32 @bar() {
        call void @foo()
        %1 = load i32* @baz, align 4
        ret i32 %1
}

