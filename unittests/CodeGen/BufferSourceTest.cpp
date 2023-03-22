//===- unittests/CodeGen/BufferSourceTest.cpp - MemoryBuffer source tests -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "latino/AST/ASTConsumer.h"
#include "latino/AST/ASTContext.h"
#include "latino/AST/RecursiveASTVisitor.h"
#include "latino/Basic/TargetInfo.h"
#include "latino/CodeGen/ModuleBuilder.h"
#include "latino/Frontend/CompilerInstance.h"
#include "latino/Lex/Preprocessor.h"
#include "latino/Parse/ParseAST.h"
#include "latino/Sema/Sema.h"
#include "llvm/ADT/Triple.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/MemoryBuffer.h"
#include "gtest/gtest.h"

using namespace llvm;
using namespace latino;

namespace {

// Emitting constructors for global objects involves looking
// at the source file name. This makes sure that we don't crash
// if the source file is a memory buffer.
const char TestProgram[] =
    "class EmitCXXGlobalInitFunc    "
    "{                              "
    "public:                        "
    "   EmitCXXGlobalInitFunc() {}  "
    "};                             "
    "EmitCXXGlobalInitFunc test;    ";

TEST(BufferSourceTest, EmitCXXGlobalInitFunc) {
    LLVMContext Context;
    CompilerInstance compiler;

    compiler.createDiagnostics();
    compiler.getLangOpts().CPlusPlus = 1;
    compiler.getLangOpts().CPlusPlus11 = 1;

    compiler.getTargetOpts().Triple = llvm::Triple::normalize(
        llvm::sys::getProcessTriple());
    compiler.setTarget(latino::TargetInfo::CreateTargetInfo(
      compiler.getDiagnostics(),
      std::make_shared<latino::TargetOptions>(
        compiler.getTargetOpts())));

    compiler.createFileManager();
    compiler.createSourceManager(compiler.getFileManager());
    compiler.createPreprocessor(latino::TU_Prefix);

    compiler.createASTContext();

    compiler.setASTConsumer(std::unique_ptr<ASTConsumer>(
        CreateLLVMCodeGen(
            compiler.getDiagnostics(),
            "EmitCXXGlobalInitFuncTest",
            compiler.getHeaderSearchOpts(),
            compiler.getPreprocessorOpts(),
            compiler.getCodeGenOpts(),
            Context)));

    compiler.createSema(latino::TU_Prefix, nullptr);

    latino::SourceManager &sm = compiler.getSourceManager();
    sm.setMainFileID(sm.createFileID(
        llvm::MemoryBuffer::getMemBuffer(TestProgram), latino::SrcMgr::C_User));

    latino::ParseAST(compiler.getSema(), false, false);
}

} // end anonymous namespace
