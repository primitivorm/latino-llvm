set(LLVM_LINK_COMPONENTS
    Support
)

add_latino_unittest(FrontendTests
    FrontendActionTest.cpp
)

latino_target_link_libraries(FrontendTests
    PRIVATE
    latinoAST
    latinoBasic
    latinoFrontend
    latinoLex
    latinoSema
    latinoSerialization
)