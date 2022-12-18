set(LLVM_LINK_COMPONENTS
  Support
  )

add_latino_unittest(LexTests
  LexerTest.cpp
  )

latino_target_link_libraries(LexTests
  PRIVATE
  latinoBasic
  latinoLex
  clangBasic
  clangLex
  )