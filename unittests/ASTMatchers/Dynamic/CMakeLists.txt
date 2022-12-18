set(LLVM_LINK_COMPONENTS
    Support
)

add_latino_unittest(DynamicASTMatchersTests
    ParserTest.cpp
)

latino_target_link_libraries(DynamicASTMatchersTests
    PRIVATE
    clangAST
    latinoBasic
)