#include "clang/Lex/PreprocessorOptions.h"

#include "latino/AST/ASTConsumer.h"
#include "latino/AST/ASTContext.h"
#include "latino/AST/Decl.h"
#include "latino/AST/RecursiveASTVisitor.h"
#include "latino/Frontend/CompilerInstance.h"
#include "latino/Frontend/FrontendAction.h"
#include "latino/Frontend/FrontendActions.h"
#include "latino/Lex/Preprocessor.h"
#include "latino/Sema/Sema.h"

#include "llvm/ADT/Triple.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ToolOutputFile.h"
#include "gtest/gtest.h"

using namespace llvm;
using namespace latino;

namespace {
class TestASTFrontendAction : public ASTFrontendAction {
public:
  TestASTFrontendAction(bool enableIncrementalProcessing = false,
                        bool actEndOfTranslationUnit = false)
      : EnableIncrementalProcessing(enableIncrementalProcessing),
        ActOnEndOfTranslationUnit(actEndOfTranslationUnit) {}

  bool EnableIncrementalProcessing;
  bool ActOnEndOfTranslationUnit;
  std::vector<std::string> decl_names;

  bool BeginSourceFileAction(CompilerInstance &ci) override {
    if (EnableIncrementalProcessing)
      ci.getPreprocessor().enableIncrementalProcessing();
    return ASTFrontendAction::BeginSourceFileAction(ci);
  }

  std::unique_ptr<ASTConsumer>
  CreateASTConsumer(CompilerInstance &CI, llvm::StringRef InFile) override {
    return std::make_unique<Visitor>(CI, ActOnEndOfTranslationUnit, decl_names);
  }

private:
  class Visitor : public ASTConsumer, public RecursiveASTVisitor<Visitor> {
  public:
    Visitor(CompilerInstance &CI, bool ActOnEndOfTranslationUnit,
            std::vector<std::string> &decl_names)
        : CI(CI), ActOnEndOfTranslationUnit(ActOnEndOfTranslationUnit),
          decl_names_(decl_names) {}

    void HandleTranslationUnit(ASTContext &context) override {
      if (ActOnEndOfTranslationUnit) {
        CI.getSema().ActOnEndOfTranslationUnit();
      }
      TraverseDecl(context.getTranslationUnitDecl());
    }

    virtual bool VisitNamedDecl(NamedDecl *Decl) {
      decl_names_.push_back(Decl->getQualifiedNameAsString());
      return true;
    }

  private:
    CompilerInstance &CI;
    bool ActOnEndOfTranslationUnit;
    std::vector<std::string> &decl_names_;
  };
};

TEST(ASTFrontendAction, Sanity) {
  auto invocation = std::make_shared<CompilerInvocation>();
  invocation->getPreprocessorOpts().addRemappedFile(
      "test.cc",
      MemoryBuffer::getMemBuffer("int main() { float x: }").release());
  invocation->getFrontendOpts().Inputs.push_back(
      FrontendInputFile("test.lat", Language::Latino));
  invocation->getFrontendOpts().ProgramAction = frontend::ParseSyntaxOnly;
  invocation->getTargetOpts().Triple = "i386-unknown-linux-gnu";
  CompilerInstance compiler;
  compiler.setInvocation(std::move(invocation));
  // compiler.createDiagnostics();

  TestASTFrontendAction test_action;
  ASSERT_TRUE(compiler.ExecuteAction(test_action));
  ASSERT_EQ(2U, test_action.decl_names.size());
  ASSERT_EQ("main", test_action.decl_names[0]);
  ASSERT_EQ("x", test_action.decl_names[1]);
}

} // namespace
