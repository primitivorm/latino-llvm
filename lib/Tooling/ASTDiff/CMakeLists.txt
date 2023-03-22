set(LLVM_LINK_COMPONENTS
  Support
  )

add_latino_library(latinoToolingASTDiff
  ASTDiff.cpp
  LINK_LIBS
  latinoBasic
  latinoAST
  latinoLex

  DEPENDS
  omp_gen
  )
