; RUN: llvm-as < %s | llc -march=mico32

define float @test_f32(float %a1, float %a2) {
        ret float %a2
}

