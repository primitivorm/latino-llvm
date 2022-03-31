set(LLVM_LINK_COMPONENTS
    FrontendOpenMP
    MC
    MCParser
    Support
)

add_clang_library(latinoParse
    ParseInit.cpp
    ParseDecl.cpp
    ParseStmt.cpp
    ParseExpr.cpp
    Parser.cpp
    ParseAST.cpp

    LINK_LIBS
    clangAST
    clangBasic
    clangSema
    latinoBasic
    latinoLex
)

