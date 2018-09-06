set(LLVM_LINK_COMPONENTS Support)
add_latino_library(latinoBasic
  Diagnostic.cpp
  DiagnosticIDs.cpp
  DiagnosticOptions.cpp
  IdentifierTable.cpp
  SourceLocation.cpp
  SourceManager.cpp
  LangOptions.cpp
  MemoryBufferCache.cpp

  LINK_LIBS
  ${LIBS}
  #clangBasic
)
