; RUN: llvm-as < %s | llc -march=arm | grep .weak
; END.
; RUN: llvm-as < %s | llc -march=lm32 | grep .weak
; DONTRUN: llvm-as < %s | llc -march=arm | grep .weak

define i32 @f(i32 %a) {
entry:
	%tmp2 = icmp eq i32 %a, 0		; <i1> [#uses=1]
	%t.0 = select i1 %tmp2, i32 (...)* null, i32 (...)* @test_weak		; <i32 (...)*> [#uses=2]
	%tmp5 = icmp eq i32 (...)* %t.0, null		; <i1> [#uses=1]
	br i1 %tmp5, label %UnifiedReturnBlock, label %cond_true8

cond_true8:		; preds = %entry
	%tmp10 = call i32 (...)* %t.0( )		; <i32> [#uses=1]
	ret i32 %tmp10

UnifiedReturnBlock:		; preds = %entry
	ret i32 250
}

declare extern_weak i32 @test_weak(...)
