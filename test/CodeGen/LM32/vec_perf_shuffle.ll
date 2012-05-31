; RUN: llvm-as < %s | llc -march=lm32
; END.
; RUN: llvm-as < %s | llc -march=ppc32 -mcpu=g5 | not grep vperm

define <2 x float> @test_uu72(<2 x float>* %P1, <2 x float>* %P2) {
	%V1 = load <2 x float>* %P1		; <<2 x float>> [#uses=1]
	%V2 = load <2 x float>* %P2		; <<2 x float>> [#uses=1]
	%V3 = shufflevector <2 x float> %V1, <2 x float> %V2, <2 x i32> < i32 undef, i32 undef >		; <<2 x float>> [#uses=1]
	ret <2 x float> %V3
}

define <2 x float> @test_30u5(<2 x float>* %P1, <2 x float>* %P2) {
	%V1 = load <2 x float>* %P1		; <<2 x float>> [#uses=1]
	%V2 = load <2 x float>* %P2		; <<2 x float>> [#uses=1]
	%V3 = shufflevector <2 x float> %V1, <2 x float> %V2, <2 x i32> < i32 3, i32 0 >		; <<2 x float>> [#uses=1]
	ret <2 x float> %V3
}

define <2 x float> @test_3u73(<2 x float>* %P1, <2 x float>* %P2) {
	%V1 = load <2 x float>* %P1		; <<2 x float>> [#uses=1]
	%V2 = load <2 x float>* %P2		; <<2 x float>> [#uses=1]
	%V3 = shufflevector <2 x float> %V1, <2 x float> %V2, <2 x i32> < i32 3, i32 undef  >		; <<2 x float>> [#uses=1]
	ret <2 x float> %V3
}

define <2 x float> @test_3772(<2 x float>* %P1, <2 x float>* %P2) {
	%V1 = load <2 x float>* %P1		; <<2 x float>> [#uses=1]
	%V2 = load <2 x float>* %P2		; <<2 x float>> [#uses=1]
	%V3 = shufflevector <2 x float> %V1, <2 x float> %V2, <2 x i32> < i32 3, i32 1 >		; <<2 x float>> [#uses=1]
	ret <2 x float> %V3
}

define <2 x float> @test_2250(<2 x float>* %P1, <2 x float>* %P2) {
	%V1 = load <2 x float>* %P1		; <<2 x float>> [#uses=1]
	%V2 = load <2 x float>* %P2		; <<2 x float>> [#uses=1]
	%V3 = shufflevector <2 x float> %V1, <2 x float> %V2, <2 x i32> < i32 2, i32 2 >		; <<2 x float>> [#uses=1]
	ret <2 x float> %V3
}
