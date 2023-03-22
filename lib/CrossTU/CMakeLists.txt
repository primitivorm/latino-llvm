set(LLVM_LINK_COMPONENTS
  Support
  )

add_latino_library(latinoCrossTU
  CrossTranslationUnit.cpp

  LINK_LIBS
  latinoAST
  latinoBasic
  latinoFrontend
  latinoIndex
  )
