#!/bin/bash
#CLANGLIBS="-lclangTooling -lclangFrontendTool -lclangFrontend -lclangDriver -lclangSerialization -lclangCodeGen -lclangParse -lclangSema -lclangStaticAnalyzerFrontend -lclangStaticAnalyzerCheckers -lclangStaticAnalyzerCore -lclangAnalysis -lclangARCMigrate -lclangRewrite -lclangRewriteFrontend -lclangEdit -lclangAST -lclangLex -lclangBasic -lclang" \
CC='clang' CXX='clang++' cmake -G "Unix Makefiles" \
-DLLVM_BUILD_TESTS=ON \
-DLATINO_INCLUDE_TESTS=ON \
-DCMAKE_BUILD_TYPE=Debug .
make
