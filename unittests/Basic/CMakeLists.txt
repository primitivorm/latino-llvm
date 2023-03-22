set(LLVM_LINK_COMPONENTS
  Support
  )

add_latino_unittest(BasicTests
  CharInfoTest.cpp
  DiagnosticTest.cpp
  FileManagerTest.cpp
  FixedPointTest.cpp
  SourceManagerTest.cpp
  )

latino_target_link_libraries(BasicTests
  PRIVATE
  latinoAST
  latinoBasic
  latinoLex
  )
