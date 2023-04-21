set(LLVM_LINK_COMPONENTS
  Support
  )

add_latino_unittest(FormatTests
  CleanupTest.cpp
  FormatTest.cpp
  FormatTestComments.cpp
  FormatTestCSharp.cpp
  FormatTestJS.cpp
  FormatTestJava.cpp
  # FormatTestObjC.cpp
  FormatTestProto.cpp
  FormatTestRawStrings.cpp
  FormatTestSelective.cpp
  FormatTestTableGen.cpp
  FormatTestTextProto.cpp
  NamespaceEndCommentsFixerTest.cpp
  SortImportsTestJS.cpp
  SortImportsTestJava.cpp
  SortIncludesTest.cpp
  UsingDeclarationsSorterTest.cpp
  )

latino_target_link_libraries(FormatTests
  PRIVATE
  latinoBasic
  latinoFormat
  latinoFrontend
  latinoRewrite
  latinoToolingCore
  )
