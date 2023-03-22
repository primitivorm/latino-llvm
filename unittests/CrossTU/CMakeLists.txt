set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  Support
  )

add_latino_unittest(CrossTUTests
  CrossTranslationUnitTest.cpp
  )

latino_target_link_libraries(CrossTUTests
  PRIVATE
  latinoAST
  latinoBasic
  latinoCrossTU
  latinoFrontend
  latinoSerialization
  latinoTooling
  )
