; mico32 supports returning <2 x i32> at most.
; trunc.ll has unsupported versions of this test.
; RUN: llvm-as -o - %s | llc -march=mico32
; END.
; RUN: llvm-as -o - %s | llc -march=cellspu > %t1.s
; RUN: grep shufb   %t1.s | count 19
; RUN: grep {ilhu.*1799}  %t1.s | count 1
; RUN: grep {ilhu.*771}  %t1.s | count 2
; RUN: grep {ilhu.*1543}  %t1.s | count 1
; RUN: grep {ilhu.*1029}  %t1.s | count 1
; RUN: grep {ilhu.*515}  %t1.s | count 1
; RUN: grep {ilhu.*3855}  %t1.s | count 1
; RUN: grep {ilhu.*3599}  %t1.s | count 1
; RUN: grep {ilhu.*3085}  %t1.s | count 1
; RUN: grep {iohl.*3855}  %t1.s | count 1
; RUN: grep {iohl.*3599}  %t1.s | count 2
; RUN: grep {iohl.*1543}  %t1.s | count 2
; RUN: grep {iohl.*771}  %t1.s | count 2
; RUN: grep {iohl.*515}  %t1.s | count 1
; RUN: grep {iohl.*1799}  %t1.s | count 1
; RUN: grep lqa  %t1.s | count 1
; RUN: grep cbd  %t1.s | count 4
; RUN: grep chd  %t1.s | count 3
; RUN: grep cwd  %t1.s | count 1
; RUN: grep cdd  %t1.s | count 1

; ModuleID = 'trunc.bc'

define i32 @trunc_i64_i32(i64 %u) {
entry:
    %0 = trunc i64 %u to i32
    ret i32 %0
}
