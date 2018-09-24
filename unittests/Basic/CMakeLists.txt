#set(LLVM_LINK_COMPONENTS Support)

add_latino_unittest(BasicTests
  DiagnosticTest.cpp
  CharInfoTest.cpp
  FileManagerTest.cpp
)

target_link_libraries(BasicTests
  PRIVATE
  latinoBasic
)
#gtest
#${GTEST_BOTH_LIBRARIES}
