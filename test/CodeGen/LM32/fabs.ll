; RUN: llvm-as < %s | llc -march=lm32
; END.
; RUN: llvm-as < %s | llc -march=ppc32 -mtriple=powerpc-apple-darwin | grep {fabs f1, f1}

define double @fabs(double %f) {
entry:
	%tmp2 = tail call double @fabs( double %f )		; <double> [#uses=1]
	ret double %tmp2
}
