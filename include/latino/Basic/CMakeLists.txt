macro(clang_diag_gen component)
  latino_tablegen(Diagnostic${component}Kinds.inc
    -gen-clang-diags-defs -clang-component=${component}
    SOURCE Diagnostic.td
    TARGET LatinoDiagnostic${component})
endmacro(clang_diag_gen)

clang_diag_gen(Analysis)
clang_diag_gen(AST)
clang_diag_gen(Comment)
clang_diag_gen(Common)
clang_diag_gen(CrossTU)
clang_diag_gen(Driver)
clang_diag_gen(Frontend)
clang_diag_gen(Lex)
clang_diag_gen(Parse)
clang_diag_gen(Refactoring)
clang_diag_gen(Sema)
clang_diag_gen(Serialization)
latino_tablegen(DiagnosticGroups.inc -gen-clang-diag-groups
  SOURCE Diagnostic.td
  TARGET LatinoDiagnosticGroups)

latino_tablegen(DiagnosticIndexName.inc -gen-clang-diags-index-name
  SOURCE Diagnostic.td
  TARGET LatinoDiagnosticIndexName)

latino_tablegen(AttrList.inc -gen-clang-attr-list
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  SOURCE Attr.td
  TARGET LatinoAttrList)

latino_tablegen(AttrSubMatchRulesList.inc -gen-clang-attr-subject-match-rule-list
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  SOURCE Attr.td
  TARGET LatinoAttrSubjectMatchRuleList)

latino_tablegen(AttrHasAttributeImpl.inc -gen-clang-attr-has-attribute-impl
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  SOURCE Attr.td
  TARGET LatinoAttrHasAttributeImpl
  )

# ARM NEON and MVE
latino_tablegen(arm_neon.inc -gen-arm-neon-sema
  SOURCE arm_neon.td
  TARGET LatinoARMNeon)
latino_tablegen(arm_fp16.inc -gen-arm-neon-sema
  SOURCE arm_fp16.td
  TARGET LatinoARMFP16)
latino_tablegen(arm_mve_builtins.inc -gen-arm-mve-builtin-def
  SOURCE arm_mve.td
  TARGET LatinoARMMveBuiltinsDef)
latino_tablegen(arm_mve_builtin_cg.inc -gen-arm-mve-builtin-codegen
  SOURCE arm_mve.td
  TARGET LatinoARMMveBuiltinCG)
latino_tablegen(arm_mve_builtin_sema.inc -gen-arm-mve-builtin-sema
  SOURCE arm_mve.td
  TARGET LatinoARMMveBuiltinSema)
latino_tablegen(arm_mve_builtin_aliases.inc -gen-arm-mve-builtin-aliases
  SOURCE arm_mve.td
  TARGET LatinoARMMveBuiltinAliases)
latino_tablegen(arm_sve_builtins.inc -gen-arm-sve-builtins
  SOURCE arm_sve.td
  TARGET LatinoARMSveBuiltins)
latino_tablegen(arm_sve_builtin_cg.inc -gen-arm-sve-builtin-codegen
  SOURCE arm_sve.td
  TARGET LatinoARMSveBuiltinCG)
latino_tablegen(arm_sve_typeflags.inc -gen-arm-sve-typeflags
  SOURCE arm_sve.td
  TARGET LatinoARMSveTypeFlags)
latino_tablegen(arm_sve_sema_rangechecks.inc -gen-arm-sve-sema-rangechecks
  SOURCE arm_sve.td
  TARGET LatinoARMSveSemaRangeChecks)
latino_tablegen(arm_cde_builtins.inc -gen-arm-cde-builtin-def
  SOURCE arm_cde.td
  TARGET LatinoARMCdeBuiltinsDef)
latino_tablegen(arm_cde_builtin_cg.inc -gen-arm-cde-builtin-codegen
  SOURCE arm_cde.td
  TARGET LatinoARMCdeBuiltinCG)
latino_tablegen(arm_cde_builtin_sema.inc -gen-arm-cde-builtin-sema
  SOURCE arm_cde.td
  TARGET LatinoARMCdeBuiltinSema)
latino_tablegen(arm_cde_builtin_aliases.inc -gen-arm-cde-builtin-aliases
  SOURCE arm_cde.td
  TARGET LatinoARMCdeBuiltinAliases)
