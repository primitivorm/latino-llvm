set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  Support
)

add_latino_library(latinoTransformer
  Parsing.cpp
  RangeSelector.cpp
  RewriteRule.cpp
  SourceCode.cpp
  SourceCodeBuilders.cpp
  Stencil.cpp
  Transformer.cpp

  LINK_LIBS
  latinoAST
  latinoASTMatchers
  latinoBasic
  latinoLex
  latinoToolingCore
  latinoToolingRefactoring

  DEPENDS
  omp_gen
  )
