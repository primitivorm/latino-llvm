set(LLVM_LINK_COMPONENTS Support)

add_latino_library(latinoToolingRefactoring
  ASTSelection.cpp
  ASTSelectionRequirements.cpp
  AtomicChange.cpp
  Extract/Extract.cpp
  Extract/SourceExtraction.cpp
  RefactoringActions.cpp
  Rename/RenamingAction.cpp
  Rename/SymbolOccurrences.cpp
  Rename/USRFinder.cpp
  Rename/USRFindingAction.cpp
  Rename/USRLocFinder.cpp

  LINK_LIBS
  latinoAST
  latinoASTMatchers
  latinoBasic
  latinoFormat
  latinoIndex
  latinoLex
  latinoRewrite
  latinoToolingCore

  DEPENDS
  omp_gen
  )
