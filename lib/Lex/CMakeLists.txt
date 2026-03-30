# TODO: Add -maltivec when ARCH is PowerPC.

set(LLVM_LINK_COMPONENTS Support)

add_clang_library(latinoLex  
  Lexer.cpp
  PPCaching.cpp
  Preprocessor.cpp
  HeaderSearch.cpp
  PreprocessorLexer.cpp
  PPLexerChange.cpp

  LINK_LIBS
  clangBasic
  )