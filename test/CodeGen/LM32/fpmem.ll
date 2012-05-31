; RUN: llvm-as < %s | llc -march=mico32
; END.

define float @f1(float %a) {
        ret float 0.000000e+00
}

define float @f2(float* %v, float %u) {
        %tmp = load float* %v           ; <float> [#uses=1]
        %tmp1 = fadd float %tmp, %u              ; <float> [#uses=1]
        ret float %tmp1
}

define void @f3(float %a, float %b, float* %v) {
        %tmp = fadd float %a, %b         ; <float> [#uses=1]
        store float %tmp, float* %v
        ret void
}
