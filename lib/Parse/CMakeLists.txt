set(LLVM_LINK_COMPONENTS
#   FrontendOpenMP
  MC
  MCParser
  Support
  )

 add_clang_library(latinoParse
  ParseAST.cpp 
   
  LINK_LIBS
  latinoAST
  latinoBasic
  latinoLex
  latinoSema

#   DEPENDS
#   omp_gen
  )