set(LLVM_LINK_COMPONENTS
  BitReader
  BitstreamReader
  Support
  )


add_latino_library(latinoSerialization
  ASTCommon.cpp
  ASTReader.cpp
  ASTReaderDecl.cpp
  ASTReaderStmt.cpp
  ASTWriter.cpp
  ASTWriterDecl.cpp
  ASTWriterStmt.cpp
  GeneratePCH.cpp
  GlobalModuleIndex.cpp
  InMemoryModuleCache.cpp
  ModuleFile.cpp
  ModuleFileExtension.cpp
  ModuleManager.cpp
  PCHContainerOperations.cpp

  ADDITIONAL_HEADERS
  ASTCommon.h
  ASTReaderInternals.h

  LINK_LIBS
  latinoAST
  latinoBasic
  latinoLex
  latinoSema

  DEPENDS
  omp_gen
  )
