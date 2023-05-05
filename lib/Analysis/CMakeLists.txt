set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  Support
  )

add_latino_library(latinoAnalysis
  AnalysisDeclContext.cpp
  BodyFarm.cpp
  CFG.cpp
  CFGReachabilityAnalysis.cpp
  CFGStmtMap.cpp
  CallGraph.cpp
  CloneDetection.cpp
  # CocoaConventions.cpp
  ConstructionContext.cpp
  Consumed.cpp
  CodeInjector.cpp
  Dominators.cpp
  ExprMutationAnalyzer.cpp
  LiveVariables.cpp
  # ObjCNoReturn.cpp
  PathDiagnostic.cpp
  PostOrderCFGView.cpp
  ProgramPoint.cpp
  ReachableCode.cpp
  # RetainSummaryManager.cpp
  ThreadSafety.cpp
  ThreadSafetyCommon.cpp
  ThreadSafetyLogical.cpp
  ThreadSafetyTIL.cpp
  UninitializedValues.cpp

  LINK_LIBS
  latinoAST
  latinoASTMatchers
  latinoBasic
  latinoLex

  DEPENDS
  omp_gen
  )

add_subdirectory(plugins)
