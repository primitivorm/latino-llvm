set(LLVM_LINK_COMPONENTS
  Core
  Support
  )

add_latino_library(latinoIndex
  CommentToXML.cpp
  FileIndexRecord.cpp
  IndexBody.cpp
  IndexDecl.cpp
  IndexingAction.cpp
  IndexingContext.cpp
  IndexSymbol.cpp
  IndexTypeSourceInfo.cpp
  USRGeneration.cpp

  ADDITIONAL_HEADERS
  IndexingContext.h
  SimpleFormatContext.h

  LINK_LIBS
  latinoAST
  latinoBasic
  latinoFormat
  latinoFrontend
  latinoLex
  latinoRewrite
  latinoSerialization
  latinoToolingCore

  DEPENDS
  omp_gen
  )
