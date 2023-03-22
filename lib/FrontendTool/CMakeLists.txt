set(LLVM_LINK_COMPONENTS
  Option
  Support
  )

set(link_libs
  latinoBasic
  latinoCodeGen
  latinoDriver
  latinoFrontend
  latinoRewriteFrontend
  )

if(CLANG_ENABLE_ARCMT)
  list(APPEND link_libs
    clangARCMigrate
    )
endif()

if(CLANG_ENABLE_STATIC_ANALYZER)
  list(APPEND link_libs
    latinoStaticAnalyzerFrontend
    )
endif()

add_latino_library(latinoFrontendTool
  ExecuteCompilerInvocation.cpp

  # DEPENDS
  # ClangDriverOptions

  LINK_LIBS
  ${link_libs}
  )
