set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  Support
  )

add_latino_unittest(DynamicASTMatchersTests
  VariantValueTest.cpp
  ParserTest.cpp
  RegistryTest.cpp
  )

latino_target_link_libraries(DynamicASTMatchersTests
  PRIVATE
  latinoAST
  latinoASTMatchers
  latinoBasic
  latinoDynamicASTMatchers
  latinoFrontend
  latinoSerialization
  latinoTesting
  latinoTooling
  )
