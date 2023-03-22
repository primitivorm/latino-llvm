set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  Support
  )

add_latino_unittest(LatinoAnalysisTests
  CFGDominatorTree.cpp
  CFGTest.cpp
  CloneDetectionTest.cpp
  ExprMutationAnalyzerTest.cpp
  )

latino_target_link_libraries(LatinoAnalysisTests
  PRIVATE
  latinoAnalysis
  latinoAST
  latinoASTMatchers
  latinoBasic
  latinoFrontend
  latinoSerialization
  latinoTooling
  )
