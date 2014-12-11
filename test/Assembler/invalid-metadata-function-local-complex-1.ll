; RUN: not llvm-as < %s -disable-output 2>&1 | FileCheck %s

define void @foo(i32 %v) {
entry:
; CHECK: <stdin>:[[@LINE+1]]:{{[0-9]+}}: error: unexpected operand after function-local metadata
  call void @llvm.bar(metadata !{i32 %v, i32 0})
  ret void
}

declare void @llvm.bar(metadata)
