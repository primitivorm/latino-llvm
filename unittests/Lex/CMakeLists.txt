set(LLVM_LINK_COMPONENTS Support)

add_latino_unittest(LexTests LexerTest.cpp)

target_link_libraries(LexTests PRIVATE
  latinoBasic
  gtest
  ${GTEST_BOTH_LIBRARIES}
)
