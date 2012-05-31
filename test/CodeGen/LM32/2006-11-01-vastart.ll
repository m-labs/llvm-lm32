; RUN: llvm-as < %s | llc -march=mico32
; END.

        %struct.va_list = type { i8*, i32, i32 }

define void @yyerror(i32, ...) {
entry:
        %va.upgrd.1 = bitcast %struct.va_list* null to i8*              ; <i8*> [#uses=1]
        call void @llvm.va_start( i8* %va.upgrd.1 )
        ret void
}

declare void @llvm.va_start(i8*)

