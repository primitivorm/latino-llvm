set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  FrontendOpenMP
  Support
  )

# By default MSVC has a 2^16 limit on the number of sections in an object file,
# and this needs more than that.
if (MSVC)
  set_source_files_properties(RecursiveASTVisitorTest.cpp PROPERTIES COMPILE_FLAGS /bigobj)
  set_source_files_properties(RecursiveASTVisitorTestExprVisitor.cpp PROPERTIES COMPILE_FLAGS /bigobj)
  set_source_files_properties(RecursiveASTVisitorTests/Callbacks.cpp PROPERTIES COMPILE_FLAGS /bigobj)
  set_source_files_properties(SourceCodeTest.cpp PROPERTIES COMPILE_FLAGS /bigobj)
endif()

add_latino_unittest(ToolingTests
  # ASTSelectionTest.cpp
  CastExprTest.cpp
  CommentHandlerTest.cpp
  CompilationDatabaseTest.cpp
  DependencyScannerTest.cpp
  DiagnosticsYamlTest.cpp
  ExecutionTest.cpp
  FixItTest.cpp
  HeaderIncludesTest.cpp
  LexicallyOrderedRecursiveASTVisitorTest.cpp
  LookupTest.cpp
  QualTypeNamesTest.cpp
  RangeSelectorTest.cpp
  RecursiveASTVisitorTests/Attr.cpp
  RecursiveASTVisitorTests/Callbacks.cpp
  RecursiveASTVisitorTests/Class.cpp
  RecursiveASTVisitorTests/ConstructExpr.cpp
  RecursiveASTVisitorTests/CXXBoolLiteralExpr.cpp
  RecursiveASTVisitorTests/CXXMemberCall.cpp
  RecursiveASTVisitorTests/CXXMethodDecl.cpp
  RecursiveASTVisitorTests/CXXOperatorCallExprTraverser.cpp
  RecursiveASTVisitorTests/DeclRefExpr.cpp
  RecursiveASTVisitorTests/ImplicitCtor.cpp
  RecursiveASTVisitorTests/ImplicitCtorInitializer.cpp
  RecursiveASTVisitorTests/InitListExprPostOrder.cpp
  RecursiveASTVisitorTests/InitListExprPostOrderNoQueue.cpp
  RecursiveASTVisitorTests/InitListExprPreOrder.cpp
  RecursiveASTVisitorTests/InitListExprPreOrderNoQueue.cpp
  RecursiveASTVisitorTests/IntegerLiteral.cpp
  RecursiveASTVisitorTests/LambdaDefaultCapture.cpp
  RecursiveASTVisitorTests/LambdaExpr.cpp
  RecursiveASTVisitorTests/LambdaTemplateParams.cpp
  RecursiveASTVisitorTests/MemberPointerTypeLoc.cpp
  RecursiveASTVisitorTests/NestedNameSpecifiers.cpp
  RecursiveASTVisitorTests/ParenExpr.cpp
  RecursiveASTVisitorTests/TemplateArgumentLocTraverser.cpp
  RecursiveASTVisitorTests/TraversalScope.cpp
  RecursiveASTVisitorTestDeclVisitor.cpp
  RecursiveASTVisitorTestPostOrderVisitor.cpp
  RecursiveASTVisitorTestTypeLocVisitor.cpp
  RefactoringActionRulesTest.cpp
  RefactoringCallbacksTest.cpp
  RefactoringTest.cpp
  ReplacementsYamlTest.cpp
  RewriterTest.cpp
  SourceCodeBuildersTest.cpp
  SourceCodeTest.cpp
  StencilTest.cpp
  ToolingTest.cpp
  TransformerTest.cpp
  )

latino_target_link_libraries(ToolingTests
  PRIVATE
  latinoAST
  latinoASTMatchers
  latinoBasic
  latinoFormat
  latinoFrontend
  latinoLex
  latinoRewrite
  latinoSerialization
  latinoTooling
  latinoToolingCore
  latinoToolingInclusions
  latinoToolingRefactoring
  latinoTransformer
  )

target_link_libraries(ToolingTests
  PRIVATE
  LLVMTestingSupport
)

add_subdirectory(Syntax)
