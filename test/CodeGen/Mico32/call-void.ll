; RUN: llvm-as < %s | llc -march=mico32
; END.
; RUN: llvm-as < %s | llc

define void @foo() {
        ret void
}

define i32 @main() {
        call void @foo( )
        ret i32 0
}

