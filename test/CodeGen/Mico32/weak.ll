; RUN: llvm-as < %s | llc -march=mico32 
; RUN: llvm-as < %s | llc -march=mico32 | grep .weak.*f
; END.
; I'm not sure how "external weak" should show up in the output,
; did on Sparc, didn't in Mips.
; RUN: llvm-as < %s | llc -march=mico32 | grep .weak.*h
;
; DONT: llvm-as < %s | llc -march=alpha | grep .weak.*f
; DONT: llvm-as < %s | llc -march=alpha | grep .weak.*h

define weak i32 @f() {
entry:
        unreachable
}

define void @g() {
entry:
        tail call void @h( )
        ret void
}

declare extern_weak void @h()

