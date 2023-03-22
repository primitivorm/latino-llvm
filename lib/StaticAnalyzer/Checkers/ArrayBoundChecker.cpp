//== ArrayBoundChecker.cpp ------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines ArrayBoundChecker, which is a path-sensitive check
// which looks for an out-of-bound array element access.
//
//===----------------------------------------------------------------------===//

#include "latino/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "latino/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "latino/StaticAnalyzer/Core/Checker.h"
#include "latino/StaticAnalyzer/Core/CheckerManager.h"
#include "latino/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "latino/StaticAnalyzer/Core/PathSensitive/DynamicSize.h"
#include "latino/StaticAnalyzer/Core/PathSensitive/ExprEngine.h"

using namespace latino;
using namespace ento;

namespace {
class ArrayBoundChecker :
    public Checker<check::Location> {
  mutable std::unique_ptr<BuiltinBug> BT;

public:
  void checkLocation(SVal l, bool isLoad, const Stmt* S,
                     CheckerContext &C) const;
};
}

void ArrayBoundChecker::checkLocation(SVal l, bool isLoad, const Stmt* LoadS,
                                      CheckerContext &C) const {
  // Check for out of bound array element access.
  const MemRegion *R = l.getAsRegion();
  if (!R)
    return;

  const ElementRegion *ER = dyn_cast<ElementRegion>(R);
  if (!ER)
    return;

  // Get the index of the accessed element.
  DefinedOrUnknownSVal Idx = ER->getIndex().castAs<DefinedOrUnknownSVal>();

  // Zero index is always in bound, this also passes ElementRegions created for
  // pointer casts.
  if (Idx.isZeroConstant())
    return;

  ProgramStateRef state = C.getState();

  // Get the size of the array.
  DefinedOrUnknownSVal ElementCount = getDynamicElementCount(
      state, ER->getSuperRegion(), C.getSValBuilder(), ER->getValueType());

  ProgramStateRef StInBound = state->assumeInBound(Idx, ElementCount, true);
  ProgramStateRef StOutBound = state->assumeInBound(Idx, ElementCount, false);
  if (StOutBound && !StInBound) {
    ExplodedNode *N = C.generateErrorNode(StOutBound);
    if (!N)
      return;

    if (!BT)
      BT.reset(new BuiltinBug(
          this, "Out-of-bound array access",
          "Access out-of-bound array element (buffer overflow)"));

    // FIXME: It would be nice to eventually make this diagnostic more clear,
    // e.g., by referencing the original declaration or by saying *why* this
    // reference is outside the range.

    // Generate a report for this bug.
    auto report =
        std::make_unique<PathSensitiveBugReport>(*BT, BT->getDescription(), N);

    report->addRange(LoadS->getSourceRange());
    C.emitReport(std::move(report));
    return;
  }

  // Array bound check succeeded.  From this point forward the array bound
  // should always succeed.
  C.addTransition(StInBound);
}

void ento::registerArrayBoundChecker(CheckerManager &mgr) {
  mgr.registerChecker<ArrayBoundChecker>();
}

bool ento::shouldRegisterArrayBoundChecker(const CheckerManager &mgr) {
  return true;
}
