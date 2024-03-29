set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  Support
  )

# By default MSVC has a 2^16 limit on the number of sections in an object file,
# and this needs more than that.
if (MSVC)
  set_source_files_properties(InternalASTMatchersTest.cpp PROPERTIES COMPILE_FLAGS /bigobj)
  set_source_files_properties(NodeMatchersTest.cpp PROPERTIES COMPILE_FLAGS /bigobj)
  set_source_files_properties(NarrowingMatchersTest.cpp PROPERTIES COMPILE_FLAGS /bigobj)
  set_source_files_properties(ASTTraversalMatchersTest.cpp PROPERTIES COMPILE_FLAGS /bigobj)
endif()

add_latino_unittest(ASTMatchersTests
  ASTMatchersInternalTest.cpp
  ASTMatchersNodeTest.cpp
  ASTMatchersNarrowingTest.cpp
  ASTMatchersTraversalTest.cpp
  GtestMatchersTest.cpp
  )

latino_target_link_libraries(ASTMatchersTests
  PRIVATE
  latinoAST
  latinoASTMatchers
  latinoBasic
  latinoFrontend
  latinoSerialization
  latinoTesting
  latinoTooling
  )

target_link_libraries(ASTMatchersTests
  PRIVATE
  LLVMTestingSupport
)

add_subdirectory(Dynamic)
