include_directories( ${CMAKE_CURRENT_BINARY_DIR}/../Checkers )

set(LLVM_LINK_COMPONENTS
  Support
  )

add_latino_library(latinoStaticAnalyzerFrontend
  AnalysisConsumer.cpp
  AnalyzerHelpFlags.cpp
  CheckerRegistry.cpp
  CreateCheckerManager.cpp
  FrontendActions.cpp
  ModelConsumer.cpp
  ModelInjector.cpp

  LINK_LIBS
  latinoAST
  latinoASTMatchers
  latinoAnalysis
  latinoBasic
  latinoCrossTU
  latinoFrontend
  latinoLex
  latinoStaticAnalyzerCheckers
  latinoStaticAnalyzerCore

  DEPENDS
  omp_gen
  )
