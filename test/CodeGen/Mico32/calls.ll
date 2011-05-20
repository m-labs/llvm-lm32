; Test various forms of calls.

; RUN: llvm-as < %s | llc -march=mico32
; DONTRUN: llvm-as < %s | llc -march=ppc32 | \
; DONTRUN:   grep {bl } | count 2
; DONTRUN: llvm-as < %s | llc -march=ppc32 | \
; DONTRUN:   grep {bctrl} | count 1
; DONTRUN: llvm-as < %s | llc -march=ppc32 | \
; DONTRUN:   grep {bla } | count 1

declare void @foo()

define void @test_direct() {
        call void @foo( )
        ret void
}

define void @test_extsym(i8* %P) {
        free i8* %P
        ret void
}

define void @test_indirect(void ()* %fp) {
        call void %fp( )
        ret void
}

define void @test_abs() {
        %fp = inttoptr i32 400 to void ()*              ; <void ()*> [#uses=1]
        call void %fp( )
        ret void
}

