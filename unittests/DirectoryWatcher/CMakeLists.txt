if(APPLE OR CMAKE_SYSTEM_NAME MATCHES "Linux")

  set(LLVM_LINK_COMPONENTS
    Support
    )

  add_latino_unittest(DirectoryWatcherTests
    DirectoryWatcherTest.cpp
    )

  target_link_libraries(DirectoryWatcherTests
    PRIVATE
    LLVMTestingSupport
    latinoDirectoryWatcher
    latinoBasic
    )

endif()
