set(LLVM_LINK_COMPONENTS Support)

add_latino_library(latinoToolingSyntax
  BuildTree.cpp
  ComputeReplacements.cpp
  Nodes.cpp
  Mutations.cpp
  Synthesis.cpp
  Tokens.cpp
  Tree.cpp

  LINK_LIBS
  latinoAST
  latinoBasic
  latinoFrontend
  latinoLex
  latinoToolingCore

  DEPENDS
  omp_gen
  )
