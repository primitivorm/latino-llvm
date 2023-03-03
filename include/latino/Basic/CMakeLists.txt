macro(latino_diag_gen component)
  latino_tablegen(Diagnostic${component}Kinds.inc
    -gen-clang-diags-defs -clang-component=${component}
    SOURCE Diagnostic.td
    TARGET ClangDiagnostic${component})
endmacro(latino_diag_gen)

latino_diag_gen(Analysis)
latino_diag_gen(AST)
latino_diag_gen(Comment)
latino_diag_gen(Common)
latino_diag_gen(CrossTU)
latino_diag_gen(Driver)
latino_diag_gen(Frontend)
latino_diag_gen(Lex)
latino_diag_gen(Parse)
latino_diag_gen(Refactoring)
latino_diag_gen(Sema)
latino_diag_gen(Serialization)
latino_tablegen(DiagnosticGroups.inc -gen-clang-diag-groups
  SOURCE Diagnostic.td
  TARGET ClangDiagnosticGroups)

latino_tablegen(DiagnosticIndexName.inc -gen-clang-diags-index-name
  SOURCE Diagnostic.td
  TARGET ClangDiagnosticIndexName)

latino_tablegen(AttrList.inc -gen-clang-attr-list
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  SOURCE Attr.td
  TARGET ClangAttrList)

latino_tablegen(AttrSubMatchRulesList.inc -gen-clang-attr-subject-match-rule-list
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  SOURCE Attr.td
  TARGET ClangAttrSubjectMatchRuleList)

latino_tablegen(AttrHasAttributeImpl.inc -gen-clang-attr-has-attribute-impl
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  SOURCE Attr.td
  TARGET ClangAttrHasAttributeImpl
  )

# ARM NEON and MVE
latino_tablegen(arm_neon.inc -gen-arm-neon-sema
  SOURCE arm_neon.td
  TARGET ClangARMNeon)
latino_tablegen(arm_fp16.inc -gen-arm-neon-sema
  SOURCE arm_fp16.td
  TARGET ClangARMFP16)
latino_tablegen(arm_mve_builtins.inc -gen-arm-mve-builtin-def
  SOURCE arm_mve.td
  TARGET ClangARMMveBuiltinsDef)
latino_tablegen(arm_mve_builtin_cg.inc -gen-arm-mve-builtin-codegen
  SOURCE arm_mve.td
  TARGET ClangARMMveBuiltinCG)
latino_tablegen(arm_mve_builtin_sema.inc -gen-arm-mve-builtin-sema
  SOURCE arm_mve.td
  TARGET ClangARMMveBuiltinSema)
latino_tablegen(arm_mve_builtin_aliases.inc -gen-arm-mve-builtin-aliases
  SOURCE arm_mve.td
  TARGET ClangARMMveBuiltinAliases)
latino_tablegen(arm_sve_builtins.inc -gen-arm-sve-builtins
  SOURCE arm_sve.td
  TARGET ClangARMSveBuiltins)
latino_tablegen(arm_sve_builtin_cg.inc -gen-arm-sve-builtin-codegen
  SOURCE arm_sve.td
  TARGET ClangARMSveBuiltinCG)
latino_tablegen(arm_sve_typeflags.inc -gen-arm-sve-typeflags
  SOURCE arm_sve.td
  TARGET ClangARMSveTypeFlags)
latino_tablegen(arm_sve_sema_rangechecks.inc -gen-arm-sve-sema-rangechecks
  SOURCE arm_sve.td
  TARGET ClangARMSveSemaRangeChecks)
latino_tablegen(arm_cde_builtins.inc -gen-arm-cde-builtin-def
  SOURCE arm_cde.td
  TARGET ClangARMCdeBuiltinsDef)
latino_tablegen(arm_cde_builtin_cg.inc -gen-arm-cde-builtin-codegen
  SOURCE arm_cde.td
  TARGET ClangARMCdeBuiltinCG)
latino_tablegen(arm_cde_builtin_sema.inc -gen-arm-cde-builtin-sema
  SOURCE arm_cde.td
  TARGET ClangARMCdeBuiltinSema)
latino_tablegen(arm_cde_builtin_aliases.inc -gen-arm-cde-builtin-aliases
  SOURCE arm_cde.td
  TARGET ClangARMCdeBuiltinAliases)
