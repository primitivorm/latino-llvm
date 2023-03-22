//===- unittest/AST/HasSideEffectsTest.cpp --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "latino/AST/RecursiveASTVisitor.h"
#include "latino/AST/ASTConsumer.h"
#include "latino/AST/ASTContext.h"
#include "latino/AST/Attr.h"
#include "latino/Frontend/FrontendAction.h"
#include "latino/Tooling/Tooling.h"
#include "llvm/ADT/FunctionExtras.h"
#include "llvm/ADT/STLExtras.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <cassert>

using namespace latino;

namespace {
class ProcessASTAction : public latino::ASTFrontendAction {
public:
  ProcessASTAction(llvm::unique_function<void(latino::ASTContext &)> Process)
      : Process(std::move(Process)) {
    assert(this->Process);
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) {
    class Consumer : public ASTConsumer {
    public:
      Consumer(llvm::function_ref<void(ASTContext &CTx)> Process)
          : Process(Process) {}

      void HandleTranslationUnit(ASTContext &Ctx) override { Process(Ctx); }

    private:
      llvm::function_ref<void(ASTContext &CTx)> Process;
    };

    return std::make_unique<Consumer>(Process);
  }

private:
  llvm::unique_function<void(latino::ASTContext &)> Process;
};

class RunHasSideEffects
    : public RecursiveASTVisitor<RunHasSideEffects> {
public:
  RunHasSideEffects(ASTContext& Ctx)
  : Ctx(Ctx) {}

  bool VisitLambdaExpr(LambdaExpr *LE) {
    LE->HasSideEffects(Ctx);
    return true;
  }

  ASTContext& Ctx;
};
} // namespace

TEST(HasSideEffectsTest, All) {
  llvm::StringRef Code = R"cpp(
void Test() {
  int msize = 4;
  float arr[msize];
  [&arr] {};
}
  )cpp";

  ASSERT_NO_FATAL_FAILURE(
    latino::tooling::runToolOnCode(
      std::make_unique<ProcessASTAction>(
          [&](latino::ASTContext &Ctx) {
              RunHasSideEffects Visitor(Ctx);
              Visitor.TraverseAST(Ctx);
          }
      ),
      Code)
  );

}
