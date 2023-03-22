set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  Support
  Option
  )

add_latino_unittest(LatinoDriverTests
  DistroTest.cpp
  ToolChainTest.cpp
  ModuleCacheTest.cpp
  MultilibTest.cpp
  SanitizerArgsTest.cpp
  )

latino_target_link_libraries(LatinoDriverTests
  PRIVATE
  latinoDriver
  latinoBasic
  latinoFrontend # For TextDiagnosticPrinter.
  )
