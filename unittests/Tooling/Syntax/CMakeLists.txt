set(LLVM_LINK_COMPONENTS
  Support
  )

add_latino_unittest(SyntaxTests
  TreeTest.cpp
  TokensTest.cpp
)

latino_target_link_libraries(SyntaxTests
  PRIVATE
  latinoAST
  latinoBasic
  latinoFrontend
  latinoLex
  latinoSerialization
  latinoTesting
  latinoTooling
  latinoToolingCore
  latinoToolingSyntax
  )

target_link_libraries(SyntaxTests
  PRIVATE
  LLVMTestingSupport
)
