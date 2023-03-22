set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  Support
  )

add_latino_unittest(SemaTests
  ExternalSemaSourceTest.cpp
  CodeCompleteTest.cpp
  GslOwnerPointerInference.cpp
  )

latino_target_link_libraries(SemaTests
  PRIVATE
  latinoAST
  latinoASTMatchers
  latinoBasic
  latinoFrontend
  latinoParse
  latinoSema
  latinoSerialization
  latinoTesting
  latinoTooling
  )

target_link_libraries(SemaTests
  PRIVATE
  LLVMTestingSupport
)
