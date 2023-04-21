set(LLVM_LINK_COMPONENTS
  Support
  )

add_latino_library(latinoRewriteFrontend
  FixItRewriter.cpp
  FrontendActions.cpp
  HTMLPrint.cpp
  InclusionRewriter.cpp
  RewriteMacros.cpp
  # RewriteModernObjC.cpp
  # RewriteObjC.cpp
  RewriteTest.cpp

  LINK_LIBS
  latinoAST
  latinoBasic
  latinoEdit
  latinoFrontend
  latinoLex
  latinoRewrite
  latinoSerialization

  DEPENDS
  omp_gen
  )
