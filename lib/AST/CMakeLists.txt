set(LINK_COMPONENTS
    BinaryFormat
    Core
    Support
)

add_latino_library(latinoAST
    APValue.cpp
    ASTConsumer.cpp
    DeclBase.cpp
    DeclGroup.cpp
    Decl.cpp
    Stmt.cpp
    StmtPrinter.cpp
)