set(LLVM_LINK_COMPONENTS
    BitReader
    BitstreamReader
    Support
)

add_latino_library(latinoSerialization
    GlobalModuleIndex.cpp

    LINK_LIBS
    latinoAST
    latinoBasic
    latinoLex
    latinoSema
)