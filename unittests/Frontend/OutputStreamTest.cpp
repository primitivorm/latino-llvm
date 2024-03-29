//===- unittests/Frontend/OutputStreamTest.cpp --- FrontendAction tests --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "latino/Basic/LangStandard.h"
#include "latino/CodeGen/BackendUtil.h"
#include "latino/CodeGen/CodeGenAction.h"
#include "latino/Frontend/CompilerInstance.h"
#include "latino/Frontend/TextDiagnosticPrinter.h"
#include "latino/FrontendTool/Utils.h"
#include "latino/Lex/PreprocessorOptions.h"
#include "gtest/gtest.h"

using namespace llvm;
using namespace latino;
using namespace latino::frontend;

namespace {

TEST(FrontendOutputTests, TestOutputStream) {
  auto Invocation = std::make_shared<CompilerInvocation>();
  Invocation->getPreprocessorOpts().addRemappedFile(
      "test.cc", MemoryBuffer::getMemBuffer("").release());
  Invocation->getFrontendOpts().Inputs.push_back(
      FrontendInputFile("test.cc", Language::CXX));
  Invocation->getFrontendOpts().ProgramAction = EmitBC;
  Invocation->getTargetOpts().Triple = "i386-unknown-linux-gnu";
  CompilerInstance Compiler;

  SmallVector<char, 256> IRBuffer;
  std::unique_ptr<raw_pwrite_stream> IRStream(
      new raw_svector_ostream(IRBuffer));

  Compiler.setOutputStream(std::move(IRStream));
  Compiler.setInvocation(std::move(Invocation));
  Compiler.createDiagnostics();

  bool Success = ExecuteCompilerInvocation(&Compiler);
  EXPECT_TRUE(Success);
  EXPECT_TRUE(!IRBuffer.empty());
  EXPECT_TRUE(StringRef(IRBuffer.data()).startswith("BC"));
}

TEST(FrontendOutputTests, TestVerboseOutputStreamShared) {
  auto Invocation = std::make_shared<CompilerInvocation>();
  Invocation->getPreprocessorOpts().addRemappedFile(
      "test.cc", MemoryBuffer::getMemBuffer("invalid").release());
  Invocation->getFrontendOpts().Inputs.push_back(
      FrontendInputFile("test.cc", Language::CXX));
  Invocation->getFrontendOpts().ProgramAction = EmitBC;
  Invocation->getTargetOpts().Triple = "i386-unknown-linux-gnu";
  CompilerInstance Compiler;

  std::string VerboseBuffer;
  raw_string_ostream VerboseStream(VerboseBuffer);

  Compiler.setOutputStream(std::make_unique<raw_null_ostream>());
  Compiler.setInvocation(std::move(Invocation));
  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
  Compiler.createDiagnostics(
      new TextDiagnosticPrinter(llvm::nulls(), &*DiagOpts), true);
  Compiler.setVerboseOutputStream(VerboseStream);

  bool Success = ExecuteCompilerInvocation(&Compiler);
  EXPECT_FALSE(Success);
  EXPECT_TRUE(!VerboseStream.str().empty());
  EXPECT_TRUE(StringRef(VerboseBuffer.data()).contains("errors generated"));
}

TEST(FrontendOutputTests, TestVerboseOutputStreamOwned) {
  std::string VerboseBuffer;
  bool Success;
  {
    auto Invocation = std::make_shared<CompilerInvocation>();
    Invocation->getPreprocessorOpts().addRemappedFile(
        "test.cc", MemoryBuffer::getMemBuffer("invalid").release());
    Invocation->getFrontendOpts().Inputs.push_back(
        FrontendInputFile("test.cc", Language::CXX));
    Invocation->getFrontendOpts().ProgramAction = EmitBC;
    Invocation->getTargetOpts().Triple = "i386-unknown-linux-gnu";
    CompilerInstance Compiler;

    std::unique_ptr<raw_ostream> VerboseStream =
        std::make_unique<raw_string_ostream>(VerboseBuffer);

    Compiler.setOutputStream(std::make_unique<raw_null_ostream>());
    Compiler.setInvocation(std::move(Invocation));
    IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
    Compiler.createDiagnostics(
        new TextDiagnosticPrinter(llvm::nulls(), &*DiagOpts), true);
    Compiler.setVerboseOutputStream(std::move(VerboseStream));

    Success = ExecuteCompilerInvocation(&Compiler);
  }
  EXPECT_FALSE(Success);
  EXPECT_TRUE(!VerboseBuffer.empty());
  EXPECT_TRUE(StringRef(VerboseBuffer.data()).contains("errors generated"));
}
}
