add_subdirectory(Rewrite)

set(LLVM_LINK_COMPONENTS
  BitReader
  BitstreamReader
  Option
  ProfileData
  Support
  )

set(optional_deps intrinsics_gen)
if (CLANG_BUILT_STANDALONE)
  set(optional_deps)
endif()

add_latino_library(latinoFrontend
  ASTConsumers.cpp
  ASTMerge.cpp
  ASTUnit.cpp
  ChainedDiagnosticConsumer.cpp
  ChainedIncludesSource.cpp
  CompilerInstance.cpp
  CompilerInvocation.cpp
  CreateInvocationFromCommandLine.cpp
  DependencyFile.cpp
  DependencyGraph.cpp
  DiagnosticRenderer.cpp
  FrontendAction.cpp
  FrontendActions.cpp
  FrontendOptions.cpp
  FrontendTiming.cpp
  HeaderIncludeGen.cpp
  InitHeaderSearch.cpp
  InitPreprocessor.cpp
  LayoutOverrideSource.cpp
  LogDiagnosticPrinter.cpp
  ModuleDependencyCollector.cpp
  MultiplexConsumer.cpp
  PrecompiledPreamble.cpp
  PrintPreprocessedOutput.cpp
  SerializedDiagnosticPrinter.cpp
  SerializedDiagnosticReader.cpp
  TestModuleFileExtension.cpp
  TextDiagnostic.cpp
  TextDiagnosticBuffer.cpp
  TextDiagnosticPrinter.cpp
  VerifyDiagnosticConsumer.cpp
  InterfaceStubFunctionsConsumer.cpp

  # DEPENDS
  # ClangDriverOptions
  # ${optional_deps}

  LINK_LIBS
  latinoAST
  latinoBasic
  latinoDriver
  latinoEdit
  latinoLex
  latinoParse
  latinoSema
  latinoSerialization
  )
