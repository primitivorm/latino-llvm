#include "latino/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "latino/StaticAnalyzer/Core/Checker.h"
#include "latino/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "latino/StaticAnalyzer/Frontend/CheckerRegistry.h"

using namespace latino;
using namespace ento;

namespace {
class MainCallChecker : public Checker<check::PreStmt<CallExpr>> {
  mutable std::unique_ptr<BugType> BT;

public:
  void checkPreStmt(const CallExpr *CE, CheckerContext &C) const;
};
} // end anonymous namespace

void MainCallChecker::checkPreStmt(const CallExpr *CE,
                                   CheckerContext &C) const {
  const Expr *Callee = CE->getCallee();
  const FunctionDecl *FD = C.getSVal(Callee).getAsFunctionDecl();

  if (!FD)
    return;

  // Get the name of the callee.
  IdentifierInfo *II = FD->getIdentifier();
  if (!II) // if no identifier, not a simple C function
    return;

  if (II->isStr("main")) {
    ExplodedNode *N = C.generateErrorNode();
    if (!N)
      return;

    if (!BT)
      BT.reset(new BugType(this, "call to main", "example analyzer plugin"));

    auto report =
        std::make_unique<PathSensitiveBugReport>(*BT, BT->getDescription(), N);
    report->addRange(Callee->getSourceRange());
    C.emitReport(std::move(report));
  }
}

// Register plugin!
extern "C" void clang_registerCheckers(CheckerRegistry &registry) {
  registry.addChecker<MainCallChecker>(
      "example.MainCallChecker", "Disallows calls to functions called main",
      "");
}

extern "C" const char clang_analyzerAPIVersionString[] =
    LATINO_ANALYZER_API_VERSION_STRING;
