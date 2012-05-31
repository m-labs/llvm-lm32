; RUN: llvm-as -o - %s | llc -march=lm32
; END.
; RUN: llvm-as -o - %s | llc -march=cellspu > %t1.s
; RUN: grep {lqd.*0(\$3)}   %t1.s | count 1
; RUN: grep {lqd.*16(\$3)}  %t1.s | count 1

; ModuleID = 'loads.bc'

define <2 x float> @load_v2f32_1(<2 x float>* %a) nounwind readonly {
entry:
	%tmp1 = load <2 x float>* %a
	ret <2 x float> %tmp1
}

define <2 x float> @load_v2f32_2(<2 x float>* %a) nounwind readonly {
entry:
	%arrayidx = getelementptr <2 x float>* %a, i32 1		; <<2 x float>*> [#uses=1]
	%tmp1 = load <2 x float>* %arrayidx		; <<2 x float>> [#uses=1]
	ret <2 x float> %tmp1
}
