set(LLVM_LINK_COMPONENTS
  Core
  Support
  )

add_latino_library(latinoDependencyScanning
  DependencyScanningFilesystem.cpp
  DependencyScanningService.cpp
  DependencyScanningWorker.cpp
  DependencyScanningTool.cpp
  ModuleDepCollector.cpp

  # DEPENDS
  # ClangDriverOptions

  LINK_LIBS
  latinoAST
  latinoBasic
  latinoDriver
  latinoFrontend
  latinoFrontendTool
  latinoLex
  latinoParse
  latinoSerialization
  latinoTooling
)
