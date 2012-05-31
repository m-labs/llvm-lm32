; RUN: llvm-as < %s | llc -march=mico32
; END.
; RUN: llvm-as < %s | llc -march=arm | grep bic | count 2
; RUN: llvm-as < %s | llc -march=arm -mattr=+v6,+vfp2 | \
; RUN:   grep fneg | count 2

target datalayout = "E-p:32:32:128-f32:32:128-i32:32:128-i16:16:128-i8:8:128-i1:8:128"

define float @test1(float %x, double %y) {
	%tmp = fpext float %x to double
	%tmp2 = tail call double @copysign( double %tmp, double %y )
	%tmp3 = fptrunc double %tmp2 to float
	ret float %tmp3
}

define double @test2(double %x, float %y) {
	%tmp = fpext float %y to double
	%tmp2 = tail call double @copysign( double %x, double %tmp )
	ret double %tmp2
}

declare double @copysign(double, double)
