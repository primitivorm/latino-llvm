set(LLVM_LINK_COMPONENTS support)

add_latino_library(latinoToolingInclusions
  HeaderIncludes.cpp
  IncludeStyle.cpp

  LINK_LIBS
  latinoBasic
  latinoLex
  latinoRewrite
  latinoToolingCore
  )
