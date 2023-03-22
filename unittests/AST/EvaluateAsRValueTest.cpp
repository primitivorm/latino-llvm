//===- unittests/AST/EvaluateAsRValueTest.cpp -----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// \file
// \brief Unit tests for evaluation of constant initializers.
//
//===----------------------------------------------------------------------===//

#include "latino/AST/ASTConsumer.h"
#include "latino/AST/ASTConsumer.h"
#include "latino/AST/ASTContext.h"
#include "latino/AST/RecursiveASTVisitor.h"
#include "latino/Tooling/Tooling.h"
#include "gtest/gtest.h"
#include <map>
#include <string>

using namespace latino::tooling;

namespace {
// For each variable name encountered, whether its initializer was a
// constant.
typedef std::map<std::string, bool> VarInfoMap;

/// \brief Records information on variable initializers to a map.
class EvaluateConstantInitializersVisitor
    : public latino::RecursiveASTVisitor<EvaluateConstantInitializersVisitor> {
 public:
  explicit EvaluateConstantInitializersVisitor(VarInfoMap &VarInfo)
      : VarInfo(VarInfo) {}

  /// \brief Checks that isConstantInitializer and EvaluateAsRValue agree
  /// and don't crash.
  ///
  /// For each VarDecl with an initializer this also records in VarInfo
  /// whether the initializer could be evaluated as a constant.
  bool VisitVarDecl(const latino::VarDecl *VD) {
    if (const latino::Expr *Init = VD->getInit()) {
      latino::Expr::EvalResult Result;
      bool WasEvaluated = Init->EvaluateAsRValue(Result, VD->getASTContext());
      VarInfo[VD->getNameAsString()] = WasEvaluated;
      EXPECT_EQ(WasEvaluated, Init->isConstantInitializer(VD->getASTContext(),
                                                          false /*ForRef*/));
    }
    return true;
  }

 private:
  VarInfoMap &VarInfo;
};

class EvaluateConstantInitializersAction : public latino::ASTFrontendAction {
 public:
   std::unique_ptr<latino::ASTConsumer>
   CreateASTConsumer(latino::CompilerInstance &Compiler,
                     llvm::StringRef FilePath) override {
     return std::make_unique<Consumer>();
  }

 private:
  class Consumer : public latino::ASTConsumer {
   public:
    ~Consumer() override {}

    void HandleTranslationUnit(latino::ASTContext &Ctx) override {
      VarInfoMap VarInfo;
      EvaluateConstantInitializersVisitor Evaluator(VarInfo);
      Evaluator.TraverseDecl(Ctx.getTranslationUnitDecl());
      EXPECT_EQ(2u, VarInfo.size());
      EXPECT_FALSE(VarInfo["Dependent"]);
      EXPECT_TRUE(VarInfo["Constant"]);
      EXPECT_EQ(2u, VarInfo.size());
    }
  };
};
}

TEST(EvaluateAsRValue, FailsGracefullyForUnknownTypes) {
  // This is a regression test; the AST library used to trigger assertion
  // failures because it assumed that the type of initializers was always
  // known (which is true only after template instantiation).
  std::string ModesToTest[] = {"-std=c++03", "-std=c++11", "-std=c++1y"};
  for (std::string const &Mode : ModesToTest) {
    std::vector<std::string> Args(1, Mode);
    Args.push_back("-fno-delayed-template-parsing");
    ASSERT_TRUE(runToolOnCodeWithArgs(
        std::make_unique<EvaluateConstantInitializersAction>(),
        "template <typename T>"
        "struct vector {"
        "  explicit vector(int size);"
        "};"
        "template <typename R>"
        "struct S {"
        "  vector<R> intervals() const {"
        "    vector<R> Dependent(2);"
        "    return Dependent;"
        "  }"
        "};"
        "void doSomething() {"
        "  int Constant = 2 + 2;"
        "  (void) Constant;"
        "}",
        Args));
  }
}
