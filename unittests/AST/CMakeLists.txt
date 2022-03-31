set(LLVM_LINK_COMPONENTS
    Support
)

add_clang_unittest(ASTTests
    DeclTest.cpp
)