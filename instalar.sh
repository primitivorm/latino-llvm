#!/bin/bash
#CLANGLIBS="-llatinoTooling -llatinoFrontendTool -llatinoFrontend -llatinoDriver -llatinoSerialization -llatinoCodeGen -llatinoParse -llatinoSema -llatinoStaticAnalyzerFrontend -lclangStaticAnalyzerCheckers -llatinoStaticAnalyzerCore -llatinoAnalysis -lclangARCMigrate -llatinoRewrite -llatinoRewriteFrontend -lclangEdit -llatinoAST -llatinoLex -llatinoBasic -lclang" \

CC='clang' CXX='clang++' cmake -G "Unix Makefiles" \
#CC='clang' CXX='clang++' cmake -G "Ninja" \
-DLLVM_BUILD_TESTS=ON \
-DLATINO_INCLUDE_TESTS=ON \
-DCMAKE_BUILD_TYPE=Debug .
make
#ninja
