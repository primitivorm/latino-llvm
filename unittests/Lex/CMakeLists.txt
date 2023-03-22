set(LLVM_LINK_COMPONENTS
  Support
  )

add_latino_unittest(LexTests
  DependencyDirectivesSourceMinimizerTest.cpp
  HeaderMapTest.cpp
  HeaderSearchTest.cpp
  LexerTest.cpp
  PPCallbacksTest.cpp
  PPConditionalDirectiveRecordTest.cpp
  )

latino_target_link_libraries(LexTests
  PRIVATE
  latinoAST
  latinoBasic
  latinoLex
  latinoParse
  latinoSema
  )
