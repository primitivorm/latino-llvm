# TODO: Add -maltivec when ARCH is PowerPC.

set(LLVM_LINK_COMPONENTS support)

add_latino_library(latinoLex
  DependencyDirectivesSourceMinimizer.cpp
  HeaderMap.cpp
  HeaderSearch.cpp
  Lexer.cpp
  LiteralSupport.cpp
  MacroArgs.cpp
  MacroInfo.cpp
  ModuleMap.cpp
  PPCaching.cpp
  PPCallbacks.cpp
  PPConditionalDirectiveRecord.cpp
  PPDirectives.cpp
  PPExpressions.cpp
  PPLexerChange.cpp
  PPMacroExpansion.cpp
  # Pragma.cpp
  PreprocessingRecord.cpp
  Preprocessor.cpp
  PreprocessorLexer.cpp
  ScratchBuffer.cpp
  TokenConcatenation.cpp
  TokenLexer.cpp

  LINK_LIBS
  latinoBasic
  )
