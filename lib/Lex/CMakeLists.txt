# TODO: Add -maltivec when ARCH is PowerPC.

set(LLVM_LINK_COMPONENTS support)

add_clang_library(latinoLex
  Lexer.cpp

  LINK_LIBS
  clangBasic
  )