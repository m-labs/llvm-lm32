; This should work.  For some reason the null doesn't
; get matched.  Works for Mips and Sparc which are similar.
; $ llvm-as null.ll -o null.bc
; $ llc -march=lm32 -debug -view-sched-dags -view-legalize-types-dags null.bc > /tmp/ll
; This does work for constant "a".
; RUN: llvm-as < %s | llc -march=lm32
; END.

	@a = constant i8 0

define i8 @bork() nounwind  {
entry:

;	%tmp1 = load i8* @a
	%tmp1 = load i8* null

	ret i8 %tmp1 
}
