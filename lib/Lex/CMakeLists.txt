# TODO: Add -maltivec when ARCH is PowerPC.

set(LLVM_LINK_COMPONENTS Support)

add_latino_library(latinoLex
  HeaderSearch.cpp
  Lexer.cpp
  PPCaching.cpp
  Preprocessor.cpp
  PPLexerChange.cpp
  PreprocessorLexer.cpp

  LINK_LIBS
  clangBasic
  )