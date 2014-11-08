; RUN: llvm-as < %s | llc -march=lm32

declare void @llvm.memory.barrier( i1 , i1 , i1 , i1 , i1)

define void @test() {
	call void @llvm.memory.barrier( i1 true, i1 true, i1 true, i1 true , i1 true)
	ret void
}
