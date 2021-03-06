; RUN: llvm-as < %s | llc -march=lm32
; END.
; RUN: llvm-as < %s | llc -march=ppc32 | grep {rlwinm r3, r3, 23, 30, 30}
; PR1473

define zeroext  i8 @foo(i16 zeroext  %a) {
        %tmp2 = lshr i16 %a, 10         ; <i16> [#uses=1]
        %tmp23 = trunc i16 %tmp2 to i8          ; <i8> [#uses=1]
        %tmp4 = shl i8 %tmp23, 1                ; <i8> [#uses=1]
        %tmp5 = and i8 %tmp4, 2         ; <i8> [#uses=1]
        ret i8 %tmp5
}

