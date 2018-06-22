set(LLVM_LINK_COMPONENTS Support)
add_latino_library(latinoBasic
  Diagnostic.cpp
  DiagnosticIDs.cpp
  DiagnosticOptions.cpp
  SourceLocation.cpp
  SourceManager.cpp

  LINK_LIBS
  clangBasic)
