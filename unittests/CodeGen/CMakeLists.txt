set(LLVM_LINK_COMPONENTS
  Core
  Support
  )

add_latino_unittest(LatinoCodeGenTests
  BufferSourceTest.cpp
  CodeGenExternalTest.cpp
  IncrementalProcessingTest.cpp
  TBAAMetadataTest.cpp
  )

latino_target_link_libraries(LatinoCodeGenTests
  PRIVATE
  latinoAST
  latinoBasic
  latinoCodeGen
  latinoFrontend
  latinoLex
  latinoParse
  latinoSerialization
  )
