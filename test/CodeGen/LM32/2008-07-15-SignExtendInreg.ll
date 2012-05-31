; RUN: llvm-as < %s | llc -march=mico32
; END.
; RUN: llvm-as < %s | llc

define signext i16 @t(i16* %dct) nounwind  {
entry:
         load i16* null, align 2         ; <i16>:0 [#uses=2]
         lshr i16 %0, 11         ; <i16>:1 [#uses=0]
         trunc i16 %0 to i8              ; <i8>:2 [#uses=1]
         sext i8 %2 to i16               ; <i16>:3 [#uses=1]
         add i16 0, %3           ; <i16>:4 [#uses=1]
         sext i16 %4 to i32              ; <i32>:5 [#uses=1]
         %dcval.0.in = shl i32 %5, 0             ; <i32> [#uses=1]
         %dcval.0 = trunc i32 %dcval.0.in to i16         ; <i16>  [#uses=1]
         store i16 %dcval.0, i16* %dct, align 2
         ret i16 0
}
