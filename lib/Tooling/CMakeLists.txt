set(LLVM_LINK_COMPONENTS
  Option
  FrontendOpenMP
  Support
  )

add_subdirectory(Core)
add_subdirectory(Inclusions)
add_subdirectory(Refactoring)
add_subdirectory(ASTDiff)
add_subdirectory(Syntax)
add_subdirectory(DependencyScanning)
add_subdirectory(Transformer)

add_latino_library(latinoTooling
  AllTUsExecution.cpp
  ArgumentsAdjusters.cpp
  CommonOptionsParser.cpp
  CompilationDatabase.cpp
  Execution.cpp
  ExpandResponseFilesCompilationDatabase.cpp
  FileMatchTrie.cpp
  FixIt.cpp
  GuessTargetAndModeCompilationDatabase.cpp
  InterpolatingCompilationDatabase.cpp
  JSONCompilationDatabase.cpp
  Refactoring.cpp
  RefactoringCallbacks.cpp
  StandaloneExecution.cpp
  Tooling.cpp

  DEPENDS
  # ClangDriverOptions
  omp_gen

  LINK_LIBS
  latinoAST
  latinoASTMatchers
  latinoBasic
  latinoDriver
  latinoFormat
  latinoFrontend
  latinoLex
  latinoRewrite
  latinoSerialization
  latinoToolingCore
  )
