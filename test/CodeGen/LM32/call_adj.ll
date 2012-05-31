;All this should do is not crash
;RUN: llvm-as < %s | llc -march=mico32

target datalayout = "e-p:32:32"

define void @_ZNSt13basic_filebufIcSt11char_traitsIcEE22_M_convert_to_externalEPcl(i32 %f) {
entry:
        %tmp49 = alloca i8, i32 %f              ; <i8*> [#uses=0]
        %tmp = call i32 null( i8* null, i8* null, i8* null, i8* null, i8* null, i8* null, i8* null )               ; <i32> [#uses=0]
        ret void
}

