set(LLVM_LINK_COMPONENTS
  Support
  )

add_latino_library(latinoRewrite
  DeltaTree.cpp
  HTMLRewrite.cpp
  RewriteRope.cpp
  Rewriter.cpp
  TokenRewriter.cpp

  LINK_LIBS
  latinoBasic
  latinoLex
  )
