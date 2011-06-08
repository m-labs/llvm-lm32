; i64 is not a leagel return type.  Should be labeled as sret.
; XFAIL: *
; RUN: llvm-as < %s | llc -march=mico32

define i64 @__mulvdi3(i64 %a, i64 %b) nounwind {
entry:
	%0 = sext i64 %a to i128		; <i128> [#uses=1]
	%1 = sext i64 %b to i128		; <i128> [#uses=1]
	%2 = mul i128 %1, %0		; <i128> [#uses=2]
	%3 = lshr i128 %2, 64		; <i128> [#uses=1]
	%4 = trunc i128 %3 to i64		; <i64> [#uses=1]
	%5 = trunc i128 %2 to i64		; <i64> [#uses=1]
	%6 = icmp eq i64 %4, 0		; <i1> [#uses=1]
	br i1 %6, label %bb1, label %bb

bb:		; preds = %entry
	unreachable

bb1:		; preds = %entry
	ret i64 %5
}
