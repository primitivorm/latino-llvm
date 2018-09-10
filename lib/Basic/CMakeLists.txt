set(LLVM_LINK_COMPONENTS Support)
add_latino_library(latinoBasic
  CharInfo.cpp
  Diagnostic.cpp
  DiagnosticIDs.cpp
  DiagnosticOptions.cpp
  FileManager.cpp
  IdentifierTable.cpp
  SourceLocation.cpp
  SourceManager.cpp
  LangOptions.cpp
  MemoryBufferCache.cpp
  TokenKinds.cpp

  LINK_LIBS
  ${LIBS}
  #clangBasic
)
