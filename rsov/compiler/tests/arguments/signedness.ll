; RUN: rs2spirv_lit_driver.sh %s | FileCheck %s

; CHECK: %__rsov_increment4_inputMemTy0 = OpTypeRuntimeArray %v4uint
; CHECK: %__rsov_increment4_outputMemTy = OpTypeRuntimeArray %v4uint
; CHECK: %__rsov_uincrement4_inputMemTy0 = OpTypeRuntimeArray %v4uint
; CHECK: %__rsov_uincrement4_outputMemTy = OpTypeRuntimeArray %v4uint
; CHECK: %__rsov_increment_inputMemTy0 = OpTypeRuntimeArray %uint
; CHECK: %__rsov_increment_outputMemTy = OpTypeRuntimeArray %uint
; CHECK: %__rsov_uincrement_inputMemTy0 = OpTypeRuntimeArray %uint
; CHECK: %__rsov_uincrement_outputMemTy = OpTypeRuntimeArray %uint

; // the RS the .rs from which this .ll is generated
; #pragma version(1)
; #pragma rs java_package_name(com.android.rs.test)

; int4 RS_KERNEL increment4(int4 in)
; {
;     return in + 1;
; }

; uint4 RS_KERNEL uincrement4(uint4 in)
; {
;     return in + 1;
; }

; int RS_KERNEL increment(int in)
; {
;     return in + 1;
; }

; unsigned RS_KERNEL uincrement(unsigned in)
; {
;     return in + 1;
; }


target datalayout = "e-p:32:32-i64:64-v128:64:128-n32-S64"
target triple = "armv7-none-linux-gnueabi"

; Function Attrs: norecurse nounwind readnone
define <4 x i32> @increment4(<4 x i32> %in) local_unnamed_addr #0 {
entry:
  %add = add <4 x i32> %in, <i32 1, i32 1, i32 1, i32 1>
  ret <4 x i32> %add
}

; Function Attrs: norecurse nounwind readnone
define <4 x i32> @uincrement4(<4 x i32> %in) local_unnamed_addr #0 {
entry:
  %add = add <4 x i32> %in, <i32 1, i32 1, i32 1, i32 1>
  ret <4 x i32> %add
}

; Function Attrs: norecurse nounwind readnone
define i32 @increment(i32 %in) local_unnamed_addr #0 {
entry:
  %add = add nsw i32 %in, 1
  ret i32 %add
}

; Function Attrs: norecurse nounwind readnone
define i32 @uincrement(i32 %in) local_unnamed_addr #0 {
entry:
  %add = add i32 %in, 1
  ret i32 %add
}

attributes #0 = { norecurse nounwind readnone "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="0" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}
!\23pragma = !{!3, !4}
!\23rs_export_foreach_name = !{!5, !6, !7, !8, !9}
!\23rs_export_foreach = !{!10, !11, !11, !11, !11}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"min_enum_size", i32 4}
!2 = !{!"Android clang version 3.8.275480  (based on LLVM 3.8.275480)"}
!3 = !{!"version", !"1"}
!4 = !{!"java_package_name", !"com.android.rs.test"}
!5 = !{!"root"}
!6 = !{!"increment4"}
!7 = !{!"uincrement4"}
!8 = !{!"increment"}
!9 = !{!"uincrement"}
!10 = !{!"0"}
!11 = !{!"35"}
