//===- unittest/AST/RecursiveASTVisitorTest.cpp ---------------------------===//
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
using ::testing::ElementsAre;

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

enum class VisitEvent {
  StartTraverseFunction,
  EndTraverseFunction,
  StartTraverseAttr,
  EndTraverseAttr
};

class CollectInterestingEvents
    : public RecursiveASTVisitor<CollectInterestingEvents> {
public:
  bool TraverseFunctionDecl(FunctionDecl *D) {
    Events.push_back(VisitEvent::StartTraverseFunction);
    bool Ret = RecursiveASTVisitor::TraverseFunctionDecl(D);
    Events.push_back(VisitEvent::EndTraverseFunction);

    return Ret;
  }

  bool TraverseAttr(Attr *A) {
    Events.push_back(VisitEvent::StartTraverseAttr);
    bool Ret = RecursiveASTVisitor::TraverseAttr(A);
    Events.push_back(VisitEvent::EndTraverseAttr);

    return Ret;
  }

  std::vector<VisitEvent> takeEvents() && { return std::move(Events); }

private:
  std::vector<VisitEvent> Events;
};

std::vector<VisitEvent> collectEvents(llvm::StringRef Code) {
  CollectInterestingEvents Visitor;
  latino::tooling::runToolOnCode(
      std::make_unique<ProcessASTAction>(
          [&](latino::ASTContext &Ctx) { Visitor.TraverseAST(Ctx); }),
      Code);
  return std::move(Visitor).takeEvents();
}
} // namespace

TEST(RecursiveASTVisitorTest, AttributesInsideDecls) {
  /// Check attributes are traversed inside TraverseFunctionDecl.
  llvm::StringRef Code = R"cpp(
__attribute__((annotate("something"))) int foo() { return 10; }
  )cpp";

  EXPECT_THAT(collectEvents(Code),
              ElementsAre(VisitEvent::StartTraverseFunction,
                          VisitEvent::StartTraverseAttr,
                          VisitEvent::EndTraverseAttr,
                          VisitEvent::EndTraverseFunction));
}
