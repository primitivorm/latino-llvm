set(LLVM_LINK_COMPONENTS
  BitReader
  BitstreamReader
  Support
  )

add_latino_unittest(SerializationTests
  InMemoryModuleCacheTest.cpp
  )

latino_target_link_libraries(SerializationTests
  PRIVATE
  latinoAST
  latinoBasic
  latinoLex
  latinoSema
  latinoSerialization
  )
