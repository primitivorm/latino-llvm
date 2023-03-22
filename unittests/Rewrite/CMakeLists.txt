set(LLVM_LINK_COMPONENTS
  Support
  )

add_latino_unittest(RewriteTests
  RewriteBufferTest.cpp
  RewriterTest.cpp
  )
latino_target_link_libraries(RewriteTests
  PRIVATE
  latinoFrontend
  latinoRewrite
  latinoSerialization
  latinoTooling
  )
