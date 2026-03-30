set(LLVM_LINK_COMPONENTS
    Support
  )

add_clang_library(latinoSema

Sema.cpp

LINK_LIBS
  latinoAST
  latinoAnalysis
  latinoBasic
  latinoEdit
  latinoLex
)