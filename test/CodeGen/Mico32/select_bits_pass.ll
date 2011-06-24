; mico32 supports returning <2 x i32> at most.
; select_bits.ll has unsupported versions of this test.
; RUN: llvm-as -o - %s | llc -march=mico32
; END.
; RUN: llvm-as -o - %s | llc -march=cellspu > %t1.s
; RUN: grep selb   %t1.s | count 56

;-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
; i32
;-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~

; (or (and rC, rB), (and (not rC), rA))
define i32 @selectbits_i32_01(i32 %rA, i32 %rB, i32 %rC) {
        %C = and i32 %rC, %rB
        %A = xor i32 %rC, -1
        %B = and i32 %A, %rA
        %D = or i32 %C, %B
        ret i32 %D
}

; (or (and rB, rC), (and (not rC), rA))
define i32 @selectbits_i32_02(i32 %rA, i32 %rB, i32 %rC) {
        %C = and i32 %rB, %rC
        %A = xor i32 %rC, -1
        %B = and i32 %A, %rA
        %D = or i32 %C, %B
        ret i32 %D
}

; (or (and (not rC), rA), (and rB, rC))
define i32 @selectbits_i32_03(i32 %rA, i32 %rB, i32 %rC) {
        %A = xor i32 %rC, -1
        %B = and i32 %A, %rA
        %C = and i32 %rB, %rC
        %D = or i32 %C, %B
        ret i32 %D
}

; (or (and (not rC), rA), (and rC, rB))
define i32 @selectbits_i32_04(i32 %rA, i32 %rB, i32 %rC) {
        %A = xor i32 %rC, -1
        %B = and i32 %A, %rA
        %C = and i32 %rC, %rB
        %D = or i32 %C, %B
        ret i32 %D
}

; (or (and rC, rB), (and rA, (not rC)))
define i32 @selectbits_i32_05(i32 %rA, i32 %rB, i32 %rC) {
        %C = and i32 %rC, %rB
        %A = xor i32 %rC, -1
        %B = and i32 %rA, %A
        %D = or i32 %C, %B
        ret i32 %D
}

; (or (and rB, rC), (and rA, (not rC)))
define i32 @selectbits_i32_06(i32 %rA, i32 %rB, i32 %rC) {
        %C = and i32 %rB, %rC
        %A = xor i32 %rC, -1
        %B = and i32 %rA, %A
        %D = or i32 %C, %B
        ret i32 %D
}

; (or (and rA, (not rC)), (and rB, rC))
define i32 @selectbits_i32_07(i32 %rA, i32 %rB, i32 %rC) {
        %A = xor i32 %rC, -1
        %B = and i32 %rA, %A
        %C = and i32 %rB, %rC
        %D = or i32 %C, %B
        ret i32 %D
}

; (or (and rA, (not rC)), (and rC, rB))
define i32 @selectbits_i32_08(i32 %rA, i32 %rB, i32 %rC) {
        %A = xor i32 %rC, -1
        %B = and i32 %rA, %A
        %C = and i32 %rC, %rB
        %D = or i32 %C, %B
        ret i32 %D
}

;-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
; i16
;-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~

; (or (and rC, rB), (and (not rC), rA))
define i16 @selectbits_i16_01(i16 %rA, i16 %rB, i16 %rC) {
        %C = and i16 %rC, %rB
        %A = xor i16 %rC, -1
        %B = and i16 %A, %rA
        %D = or i16 %C, %B
        ret i16 %D
}

; (or (and rB, rC), (and (not rC), rA))
define i16 @selectbits_i16_02(i16 %rA, i16 %rB, i16 %rC) {
        %C = and i16 %rB, %rC
        %A = xor i16 %rC, -1
        %B = and i16 %A, %rA
        %D = or i16 %C, %B
        ret i16 %D
}

; (or (and (not rC), rA), (and rB, rC))
define i16 @selectbits_i16_03(i16 %rA, i16 %rB, i16 %rC) {
        %A = xor i16 %rC, -1
        %B = and i16 %A, %rA
        %C = and i16 %rB, %rC
        %D = or i16 %C, %B
        ret i16 %D
}

; (or (and (not rC), rA), (and rC, rB))
define i16 @selectbits_i16_04(i16 %rA, i16 %rB, i16 %rC) {
        %A = xor i16 %rC, -1
        %B = and i16 %A, %rA
        %C = and i16 %rC, %rB
        %D = or i16 %C, %B
        ret i16 %D
}

; (or (and rC, rB), (and rA, (not rC)))
define i16 @selectbits_i16_05(i16 %rA, i16 %rB, i16 %rC) {
        %C = and i16 %rC, %rB
        %A = xor i16 %rC, -1
        %B = and i16 %rA, %A
        %D = or i16 %C, %B
        ret i16 %D
}

; (or (and rB, rC), (and rA, (not rC)))
define i16 @selectbits_i16_06(i16 %rA, i16 %rB, i16 %rC) {
        %C = and i16 %rB, %rC
        %A = xor i16 %rC, -1
        %B = and i16 %rA, %A
        %D = or i16 %C, %B
        ret i16 %D
}

; (or (and rA, (not rC)), (and rB, rC))
define i16 @selectbits_i16_07(i16 %rA, i16 %rB, i16 %rC) {
        %A = xor i16 %rC, -1
        %B = and i16 %rA, %A
        %C = and i16 %rB, %rC
        %D = or i16 %C, %B
        ret i16 %D
}

; (or (and rA, (not rC)), (and rC, rB))
define i16 @selectbits_i16_08(i16 %rA, i16 %rB, i16 %rC) {
        %A = xor i16 %rC, -1
        %B = and i16 %rA, %A
        %C = and i16 %rC, %rB
        %D = or i16 %C, %B
        ret i16 %D
}

;-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
; i8
;-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~

; (or (and rC, rB), (and (not rC), rA))
define i8 @selectbits_i8_01(i8 %rA, i8 %rB, i8 %rC) {
        %C = and i8 %rC, %rB
        %A = xor i8 %rC, -1
        %B = and i8 %A, %rA
        %D = or i8 %C, %B
        ret i8 %D
}

; (or (and rB, rC), (and (not rC), rA))
define i8 @selectbits_i8_02(i8 %rA, i8 %rB, i8 %rC) {
        %C = and i8 %rB, %rC
        %A = xor i8 %rC, -1
        %B = and i8 %A, %rA
        %D = or i8 %C, %B
        ret i8 %D
}

; (or (and (not rC), rA), (and rB, rC))
define i8 @selectbits_i8_03(i8 %rA, i8 %rB, i8 %rC) {
        %A = xor i8 %rC, -1
        %B = and i8 %A, %rA
        %C = and i8 %rB, %rC
        %D = or i8 %C, %B
        ret i8 %D
}

; (or (and (not rC), rA), (and rC, rB))
define i8 @selectbits_i8_04(i8 %rA, i8 %rB, i8 %rC) {
        %A = xor i8 %rC, -1
        %B = and i8 %A, %rA
        %C = and i8 %rC, %rB
        %D = or i8 %C, %B
        ret i8 %D
}

; (or (and rC, rB), (and rA, (not rC)))
define i8 @selectbits_i8_05(i8 %rA, i8 %rB, i8 %rC) {
        %C = and i8 %rC, %rB
        %A = xor i8 %rC, -1
        %B = and i8 %rA, %A
        %D = or i8 %C, %B
        ret i8 %D
}

; (or (and rB, rC), (and rA, (not rC)))
define i8 @selectbits_i8_06(i8 %rA, i8 %rB, i8 %rC) {
        %C = and i8 %rB, %rC
        %A = xor i8 %rC, -1
        %B = and i8 %rA, %A
        %D = or i8 %C, %B
        ret i8 %D
}

; (or (and rA, (not rC)), (and rB, rC))
define i8 @selectbits_i8_07(i8 %rA, i8 %rB, i8 %rC) {
        %A = xor i8 %rC, -1
        %B = and i8 %rA, %A
        %C = and i8 %rB, %rC
        %D = or i8 %C, %B
        ret i8 %D
}

; (or (and rA, (not rC)), (and rC, rB))
define i8 @selectbits_i8_08(i8 %rA, i8 %rB, i8 %rC) {
        %A = xor i8 %rC, -1
        %B = and i8 %rA, %A
        %C = and i8 %rC, %rB
        %D = or i8 %C, %B
        ret i8 %D
}
