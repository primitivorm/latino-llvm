set(LLVM_LINK_COMPONENTS
    BitReader
    BitstreamReader
    Option
    ProfileData
    Support
    )

add_latino_library(latinoFrontend
    FrontendAction.cpp
    FrontendActions.cpp
    CompilerInstance.cpp

    LINK_LIBS
    latinoAST
    latinoBasic
    latinoLex
    latinoParse
    latinoSema
)