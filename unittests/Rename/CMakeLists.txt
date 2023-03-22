set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  support
  )

# We'd like clang/unittests/Tooling/RewriterTestContext.h in the test.
include_directories(${LATINO_SOURCE_DIR})

add_latino_unittest(LatinoRenameTests
  RenameClassTest.cpp
  RenameEnumTest.cpp
  RenameAliasTest.cpp
  RenameMemberTest.cpp
  RenameFunctionTest.cpp
  )

latino_target_link_libraries(LatinoRenameTests
  PRIVATE
  latinoAST
  latinoASTMatchers
  latinoBasic
  latinoFormat
  latinoFrontend
  latinoRewrite
  latinoSerialization
  latinoTooling
  latinoToolingCore
  latinoToolingRefactoring
  )
