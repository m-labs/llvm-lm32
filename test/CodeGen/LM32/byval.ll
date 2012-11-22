; RUN: llc -march=lm32 < %s | FileCheck %s
; NOTE: if the memcpy threshold is adjusted, some of those tests might
; generate a false negative. Adjust accordingly in such case.
%struct.s0 = type { [8 x i32] }
%struct.s1 = type { [9 x i32] }
%struct.s2 = type { [7 x i32] }

; Test for byval aggregate that fits in arg regs.
define void @f0_1(%struct.s0* byval %s) {
entry:
  call void @f0(%struct.s0* byval %s)
  ret void
}

declare void @f0(%struct.s0* byval)
; CHECK: f0_1:
; CHECK: addi     sp, sp, -36
; CHECK: sw       (sp+4), r1
; CHECK: calli    f0
; CHECK: addi     sp, sp, 36

; Test for byval aggregate that doesn't fit in arg regs.
define void @f1_1(%struct.s1* byval %s) {
entry:
  call void @f1(%struct.s1* byval %s)
  ret void
}

declare void @f1(%struct.s1* byval)
; CHECK: f1_1:
; CHECK: addi     sp, sp, -40
; CHECK: addi     r1, sp, 4
; CHECK: addi     r2, sp, 44
; CHECK: addi     r3, r0, 36
; CHECK: calli    memcpy
; CHECK: calli    f1
; CHECK: addi     sp, sp, 40


; Test for byval aggregate with trailing argument that fit in arg regs.
define void @f2_1(%struct.s2* byval %s) {
entry:
  call void @f2(%struct.s2* byval %s, i32 1)
  ret void
}

declare void @f2(%struct.s2* byval, i32)
; CHECK: f2_1:
; CHECK: addi     sp, sp, -32
; CHECK: sw       (sp+4), r1
; CHECK: addi     r8, r0, 1
; CHECK: calli    f2
; CHECK: addi     sp, sp, 32

; Test for byval aggregate with trailing argument that doesn't fit in arg regs.
define void @f3_1(%struct.s1* byval %s) {
entry:
  call void @f3(%struct.s1* byval %s, i32 1)
  ret void
}

declare void @f3(%struct.s1* byval, i32)
; CEHCK: f3_1:
; CHECK: addi     sp, sp, -44
; CHECK: addi     r1, sp, 4
; CHECK: addi     r2, sp, 48
; CHECK: addi     r3, r0, 36
; CHECK: calli    memcpy
; CHECK: addi     r{{[1-25]}}, r0, 1
; CHECK: sw       (sp+40), r{{[1-25]}}
; CHECK: calli    f3
; CHECK: addi     sp, sp, 44
