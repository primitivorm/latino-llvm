add_subdirectory(Dynamic)

set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  Support
)

add_latino_library(latinoASTMatchers
  ASTMatchFinder.cpp
  ASTMatchersInternal.cpp
  GtestMatchers.cpp

  LINK_LIBS
  latinoAST
  latinoBasic
  latinoLex

  DEPENDS
  omp_gen
  )
