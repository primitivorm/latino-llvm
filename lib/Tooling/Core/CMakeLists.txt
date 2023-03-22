set(LLVM_LINK_COMPONENTS support)

add_latino_library(latinoToolingCore
  Diagnostic.cpp
  Lookup.cpp
  Replacement.cpp

  LINK_LIBS
  latinoAST
  latinoBasic
  latinoLex
  latinoRewrite
  )
