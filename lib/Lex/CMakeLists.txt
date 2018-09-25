set(LLVM_LINK_COMPONENTS
  Support)

add_latino_library(latinoLex
  Lexer.cpp

  LINK_LIBS
  ${LIBS}
)
