; RUN: llvm-as < %s | llc -march=lm32

define i32* @foo(i32 %n) {
        %A = alloca i32, i32 %n         ; <i32*> [#uses=1]
        ret i32* %A
}

