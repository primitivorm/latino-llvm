#include "latino/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "latino/StaticAnalyzer/Core/Checker.h"
#include "latino/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "latino/StaticAnalyzer/Frontend/CheckerRegistry.h"

using namespace latino;
using namespace ento;

namespace {
struct Dependency : public Checker<check::BeginFunction> {
  void checkBeginFunction(CheckerContext &Ctx) const {}
};
struct DependendentChecker : public Checker<check::BeginFunction> {
  void checkBeginFunction(CheckerContext &Ctx) const {}
};
} // end anonymous namespace

// Register plugin!
extern "C" void clang_registerCheckers(CheckerRegistry &registry) {
  registry.addChecker<Dependency>("example.Dependency", "", "");
  registry.addChecker<DependendentChecker>("example.DependendentChecker", "",
                                           "");

  registry.addDependency("example.DependendentChecker", "example.Dependency");
}

extern "C" const char clang_analyzerAPIVersionString[] =
    LATINO_ANALYZER_API_VERSION_STRING;
