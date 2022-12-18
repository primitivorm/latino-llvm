set (LLVM_LINK_COMPONENTS
    FrontendOpenMP
    Support
)

# if(MSVC)
#     set_source_files_properties(SemaExpr.cpp PROPERTIES COMPILE_FLAGS /bigobj)
# endif()

add_latino_library(latinoSema
    # AnalysisBasedWarnings.cpp
    # CodeCompleteConsumer.cpp
    # DeclSpec.cpp
    # DelayedDiagnostic.cpp
    # IdentifierResolver.cpp
    # JumpDiagnostics.cpp
    # MultiplexExternalSemaSource.cpp
    # ParsedAttr.cpp
    # Scope.cpp
    # ScopeInfo.cpp
    Sema.cpp
    # SemaAccess.cpp
    # SemaAttr.cpp
    # SemaAvailability.cpp
    # SemaCXXScopeSpec.cpp
    # SemaCast.cpp
    # SemaChecking.cpp
    # SemaCodeComplete.cpp
    # SemaConcept.cpp
    # SemaConsumer.cpp
    # SemaCoroutine.cpp
    # SemaCUDA.cpp
    # SemaDecl.cpp
    # SemaDeclAttr.cpp
    # SemaDeclCXX.cpp
    # SemaDeclObjC.cpp
    # SemaExceptionSpec.cpp
    # SemaExpr.cpp
    # SemaExprCXX.cpp
    # SemaExprMember.cpp
    # SemaExprObjC.cpp
    # SemaFixItUtils.cpp
    # SemaInit.cpp
    # SemaLambda.cpp
    # SemaLookup.cpp
    # SemaModule.cpp
    # SemaObjCProperty.cpp
    # SemaOpenMP.cpp
    # SemaOverload.cpp
    # SemaPseudoObject.cpp
    SemaStmt.cpp
    # SemaStmtAsm.cpp
    # SemaStmtAttr.cpp
    # SemaSYCL.cpp
    # SemaType.cpp
    # TypeLocBuilder.cpp

    LINK_LIBS
    latinoAST
    latinoBasic
    latinoLex
)