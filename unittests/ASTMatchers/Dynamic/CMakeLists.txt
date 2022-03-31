set(LLVM_LINK_COMPONENTS
    Support
)

add_clang_unittest(DynamicASTMatchersTests
    ParserTest.cpp
)

clang_target_link_libraries(DynamicASTMatchersTests
    PRIVATE
    clangAST
    latinoBasic
)