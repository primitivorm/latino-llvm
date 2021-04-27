set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_unittest(LexTests
  LexerTest.cpp
  )

clang_target_link_libraries(LexTests
  PRIVATE
  latinoBasic
  latinoLex
  clangBasic
  clangLex
  )