; RUN: llvm-as < %s | llc -march=lm32
; END.
; RUN: llvm-as < %s | llc -march=ppc32 | grep fctiwz | count 1

define i16 @foo(float %a) {
entry:
        %tmp.1 = fptoui float %a to i16         ; <i16> [#uses=1]
        ret i16 %tmp.1
}

