add_latino_unittest(libclangCrashTests
  LibclangCrashTest.cpp
  )

target_link_libraries(libclangCrashTests
  PRIVATE
  libclang
  )
