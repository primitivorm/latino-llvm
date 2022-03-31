set(LINK_COMPONENTS
    BinaryFormat
    Core
    Support
)

add_clang_library(latinoAST
    ASTConsumer.cpp
    DeclBase.cpp
    Stmt.cpp
)