add_latino_unittest(libclangTests
  LibclangTest.cpp
  )

target_link_libraries(libclangTests
  PRIVATE
  libclang
  )

add_subdirectory(CrashTests)
