set(LLVM_LINK_COMPONENTS
  Support
  )

add_latino_library(latinoEdit
  Commit.cpp
  EditedSource.cpp
  RewriteObjCFoundationAPI.cpp

  LINK_LIBS
  latinoAST
  latinoBasic
  latinoLex
  )
