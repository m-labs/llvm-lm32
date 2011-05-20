; This should work.  For some reason the null doesn't
; get matched.  Works for Mips and Sparc which are similar.
; $ llvm-as null16.ll -o null16.bc
; $ llc -march=mico32 -debug -view-sched-dags -view-legalize-types-dags null16.bc > /tmp/ll
; RUN: llvm-as < %s | llc -march=mico32
; END.

define i16 @bork() nounwind  {
entry:
	%tmp1 = load i16* null
	ret i16 %tmp1 
}
