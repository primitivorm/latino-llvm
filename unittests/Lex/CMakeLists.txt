set(LLVM_LINK_COMPONENTS
  Support
  Target
  )

add_clang_unittest(LexTests
  LexerTest.cpp
  )

clang_target_link_libraries(LexTests
  PRIVATE
  latinoLex
  clangBasic
  clangLex
  )