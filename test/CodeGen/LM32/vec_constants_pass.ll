; RUN: llvm-as < %s | llc -march=lm32
; END.
; RUN: llvm-as < %s | llc -march=ppc32 -mcpu=g5 | not grep CPI

define void @test1(<4 x i32>* %P1, <4 x i32>* %P2, <4 x float>* %P3) {
	%tmp = load <4 x i32>* %P1		; <<4 x i32>> [#uses=1]
	%tmp4 = and <4 x i32> %tmp, < i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648 >		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp4, <4 x i32>* %P1
	%tmp7 = load <4 x i32>* %P2		; <<4 x i32>> [#uses=1]
	%tmp9 = and <4 x i32> %tmp7, < i32 2147483647, i32 2147483647, i32 2147483647, i32 2147483647 >		; <<4 x i32>> [#uses=1]
	store <4 x i32> %tmp9, <4 x i32>* %P2
	%tmp.upgrd.1 = load <4 x float>* %P3		; <<4 x float>> [#uses=1]
	%tmp11 = bitcast <4 x float> %tmp.upgrd.1 to <4 x i32>		; <<4 x i32>> [#uses=1]
	%tmp12 = and <4 x i32> %tmp11, < i32 2147483647, i32 2147483647, i32 2147483647, i32 2147483647 >		; <<4 x i32>> [#uses=1]
	%tmp13 = bitcast <4 x i32> %tmp12 to <4 x float>		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp13, <4 x float>* %P3
	ret void
}
