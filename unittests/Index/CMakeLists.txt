set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  Support
  )

add_latino_unittest(IndexTests
  IndexTests.cpp
  )

latino_target_link_libraries(IndexTests
  PRIVATE
  latinoAST
  latinoBasic
  latinoFrontend
  latinoIndex
  latinoLex
  latinoSerialization
  latinoTooling
  )
