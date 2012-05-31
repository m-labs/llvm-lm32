;test for ADDC and ADDE expansion
;
; lm32 cannot return i128.  Should be labeled as sret.
; XFAIL: *
; RUN: llvm-as < %s | llc -march=lm32

define i128 @add128(i128 %x, i128 %y) {
entry:
	%tmp = add i128 %y, %x
	ret i128 %tmp
}
