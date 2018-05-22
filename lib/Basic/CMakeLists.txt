set(LLVM_LINK_COMPONENTS Support)
add_latino_library(latinoBasic
  #IdentifierTable.cpp
  SourceLocation.cpp
  SourceManager.cpp

  LINK_LIBS
  clangBasic)
