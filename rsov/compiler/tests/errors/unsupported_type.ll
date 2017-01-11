; RUN: not rs2spirv_lit_driver.sh %s >%t.out 2>%t.err
; RUN: FileCheck %s <%t.err

target datalayout = "e-p:32:32-i64:64-v128:64:128-n32-S64"
target triple = "armv7-none-linux-gnueabi"

; CHECK: LLVM to SPIRV type mapping for type:    [i16] not found
; CHECK: Emitting kernel types for tSet failed

; CHECK-NOT: OpFunctionEnd

; Function Attrs: nounwind readnone
define signext i16 @tSet(i16 signext %v) unnamed_addr #0 {
  ret i16 %v
}

attributes #0 = { nounwind readnone }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}
!\23pragma = !{!3, !4}
!\23rs_export_foreach_name = !{!5, !6}
!\23rs_export_foreach = !{!7, !8}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"min_enum_size", i32 4}
!2 = !{!"Android clang version 3.8.275480  (based on LLVM 3.8.275480)"}
!3 = !{!"version", !"1"}
!4 = !{!"java_package_name", !"identity"}
!5 = !{!"root"}
!6 = !{!"tSet"}
!7 = !{!"0"}
!8 = !{!"35"}
