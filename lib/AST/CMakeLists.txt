set(LLVM_LINK_COMPONENTS
  BinaryFormat
  Core
#   FrontendOpenMP
  Support
  )

  add_clang_library(latinoAST
  ASTConsumer.cpp
  ASTContext.cpp
  DeclBase.cpp

  LINK_LIBS
  latinoBasic
  latinoLex

  # DEPENDS
  # Opcodes
  # omp_gen
  )