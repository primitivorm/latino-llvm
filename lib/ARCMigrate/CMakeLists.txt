set(LLVM_LINK_COMPONENTS
  Support
  )

add_latino_library(latinoARCMigrate
  ARCMT.cpp
  ARCMTActions.cpp
  FileRemapper.cpp
  # ObjCMT.cpp
  PlistReporter.cpp
  TransAPIUses.cpp
  TransARCAssign.cpp
  # TransAutoreleasePool.cpp
  # TransBlockObjCVariable.cpp
  TransEmptyStatementsAndDealloc.cpp
  TransGCAttrs.cpp
  TransGCCalls.cpp
  TransProperties.cpp
  TransProtectedScope.cpp
  TransRetainReleaseDealloc.cpp
  # TransUnbridgedCasts.cpp
  TransUnusedInitDelegate.cpp
  # TransZeroOutPropsInDealloc.cpp
  TransformActions.cpp
  Transforms.cpp

  LINK_LIBS
  latinoAST
  latinoAnalysis
  latinoBasic
  latinoEdit
  latinoFrontend
  latinoLex
  latinoRewrite
  latinoSema
  latinoSerialization

  DEPENDS
  omp_gen
  )
