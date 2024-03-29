// SmartPtrModeling.cpp - Model behavior of C++ smart pointers - C++ ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines a checker that models various aspects of
// C++ smart pointer behavior.
//
//===----------------------------------------------------------------------===//

#include "Move.h"
#include "SmartPtr.h"

#include "latino/AST/DeclCXX.h"
#include "latino/AST/ExprCXX.h"
#include "latino/AST/Type.h"
#include "latino/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "latino/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "latino/StaticAnalyzer/Core/Checker.h"
#include "latino/StaticAnalyzer/Core/CheckerManager.h"
#include "latino/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "latino/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "latino/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include "latino/StaticAnalyzer/Core/PathSensitive/SymExpr.h"

using namespace latino;
using namespace ento;

namespace {
class SmartPtrModeling : public Checker<eval::Call, check::DeadSymbols> {

  bool isNullAfterMoveMethod(const CallEvent &Call) const;

public:
  // Whether the checker should model for null dereferences of smart pointers.
  DefaultBool ModelSmartPtrDereference;
  bool evalCall(const CallEvent &Call, CheckerContext &C) const;
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
  void checkDeadSymbols(SymbolReaper &SymReaper, CheckerContext &C) const;

private:
  ProgramStateRef updateTrackedRegion(const CallEvent &Call, CheckerContext &C,
                                      const MemRegion *ThisValRegion) const;
  void handleReset(const CallEvent &Call, CheckerContext &C) const;
  void handleRelease(const CallEvent &Call, CheckerContext &C) const;
  void handleSwap(const CallEvent &Call, CheckerContext &C) const;

  using SmartPtrMethodHandlerFn =
      void (SmartPtrModeling::*)(const CallEvent &Call, CheckerContext &) const;
  CallDescriptionMap<SmartPtrMethodHandlerFn> SmartPtrMethodHandlers{
      {{"reset"}, &SmartPtrModeling::handleReset},
      {{"release"}, &SmartPtrModeling::handleRelease},
      {{"swap", 1}, &SmartPtrModeling::handleSwap}};
};
} // end of anonymous namespace

REGISTER_MAP_WITH_PROGRAMSTATE(TrackedRegionMap, const MemRegion *, SVal)

// Define the inter-checker API.
namespace latino {
namespace ento {
namespace smartptr {
bool isStdSmartPtrCall(const CallEvent &Call) {
  const auto *MethodDecl = dyn_cast_or_null<CXXMethodDecl>(Call.getDecl());
  if (!MethodDecl || !MethodDecl->getParent())
    return false;

  const auto *RecordDecl = MethodDecl->getParent();
  if (!RecordDecl || !RecordDecl->getDeclContext()->isStdNamespace())
    return false;

  if (RecordDecl->getDeclName().isIdentifier()) {
    StringRef Name = RecordDecl->getName();
    return Name == "shared_ptr" || Name == "unique_ptr" || Name == "weak_ptr";
  }
  return false;
}

bool isNullSmartPtr(const ProgramStateRef State, const MemRegion *ThisRegion) {
  const auto *InnerPointVal = State->get<TrackedRegionMap>(ThisRegion);
  return InnerPointVal && InnerPointVal->isZeroConstant();
}
} // namespace smartptr
} // namespace ento
} // namespace latino

bool SmartPtrModeling::isNullAfterMoveMethod(const CallEvent &Call) const {
  // TODO: Update CallDescription to support anonymous calls?
  // TODO: Handle other methods, such as .get() or .release().
  // But once we do, we'd need a visitor to explain null dereferences
  // that are found via such modeling.
  const auto *CD = dyn_cast_or_null<CXXConversionDecl>(Call.getDecl());
  return CD && CD->getConversionType()->isBooleanType();
}

bool SmartPtrModeling::evalCall(const CallEvent &Call,
                                CheckerContext &C) const {

  if (!smartptr::isStdSmartPtrCall(Call))
    return false;

  if (isNullAfterMoveMethod(Call)) {
    ProgramStateRef State = C.getState();
    const MemRegion *ThisR =
        cast<CXXInstanceCall>(&Call)->getCXXThisVal().getAsRegion();

    if (!move::isMovedFrom(State, ThisR)) {
      // TODO: Model this case as well. At least, avoid invalidation of globals.
      return false;
    }

    // TODO: Add a note to bug reports describing this decision.
    C.addTransition(
        State->BindExpr(Call.getOriginExpr(), C.getLocationContext(),
                        C.getSValBuilder().makeZeroVal(Call.getResultType())));
    return true;
  }

  if (!ModelSmartPtrDereference)
    return false;

  if (const auto *CC = dyn_cast<CXXConstructorCall>(&Call)) {
    if (CC->getDecl()->isCopyOrMoveConstructor())
      return false;

    const MemRegion *ThisValRegion = CC->getCXXThisVal().getAsRegion();
    if (!ThisValRegion)
      return false;

    auto State = updateTrackedRegion(Call, C, ThisValRegion);
    C.addTransition(State);
    return true;
  }

  const SmartPtrMethodHandlerFn *Handler = SmartPtrMethodHandlers.lookup(Call);
  if (!Handler)
    return false;
  (this->**Handler)(Call, C);

  return C.isDifferent();
}

void SmartPtrModeling::checkDeadSymbols(SymbolReaper &SymReaper,
                                        CheckerContext &C) const {
  ProgramStateRef State = C.getState();
  // Clean up dead regions from the region map.
  TrackedRegionMapTy TrackedRegions = State->get<TrackedRegionMap>();
  for (auto E : TrackedRegions) {
    const MemRegion *Region = E.first;
    bool IsRegDead = !SymReaper.isLiveRegion(Region);

    if (IsRegDead)
      State = State->remove<TrackedRegionMap>(Region);
  }
  C.addTransition(State);
}

void SmartPtrModeling::handleReset(const CallEvent &Call,
                                   CheckerContext &C) const {
  const auto *IC = dyn_cast<CXXInstanceCall>(&Call);
  if (!IC)
    return;

  const MemRegion *ThisValRegion = IC->getCXXThisVal().getAsRegion();
  if (!ThisValRegion)
    return;
  auto State = updateTrackedRegion(Call, C, ThisValRegion);
  C.addTransition(State);
  // TODO: Make sure to ivalidate the the region in the Store if we don't have
  // time to model all methods.
}

void SmartPtrModeling::handleRelease(const CallEvent &Call,
                                     CheckerContext &C) const {
  const auto *IC = dyn_cast<CXXInstanceCall>(&Call);
  if (!IC)
    return;

  const MemRegion *ThisValRegion = IC->getCXXThisVal().getAsRegion();
  if (!ThisValRegion)
    return;

  auto State = updateTrackedRegion(Call, C, ThisValRegion);

  const auto *InnerPointVal = State->get<TrackedRegionMap>(ThisValRegion);
  if (InnerPointVal) {
    State = State->BindExpr(Call.getOriginExpr(), C.getLocationContext(),
                            *InnerPointVal);
  }
  C.addTransition(State);
  // TODO: Add support to enable MallocChecker to start tracking the raw
  // pointer.
}

void SmartPtrModeling::handleSwap(const CallEvent &Call,
                                  CheckerContext &C) const {
  // TODO: Add support to handle swap method.
}

ProgramStateRef
SmartPtrModeling::updateTrackedRegion(const CallEvent &Call, CheckerContext &C,
                                      const MemRegion *ThisValRegion) const {
  // TODO: Refactor and clean up handling too many things.
  ProgramStateRef State = C.getState();
  auto NumArgs = Call.getNumArgs();

  if (NumArgs == 0) {
    auto NullSVal = C.getSValBuilder().makeNull();
    State = State->set<TrackedRegionMap>(ThisValRegion, NullSVal);
  } else if (NumArgs == 1) {
    auto ArgVal = Call.getArgSVal(0);
    assert(Call.getArgExpr(0)->getType()->isPointerType() &&
           "Adding a non pointer value to TrackedRegionMap");
    State = State->set<TrackedRegionMap>(ThisValRegion, ArgVal);
  }

  return State;
}

void ento::registerSmartPtrModeling(CheckerManager &Mgr) {
  auto *Checker = Mgr.registerChecker<SmartPtrModeling>();
  Checker->ModelSmartPtrDereference =
      Mgr.getAnalyzerOptions().getCheckerBooleanOption(
          Checker, "ModelSmartPtrDereference");
}

bool ento::shouldRegisterSmartPtrModeling(const CheckerManager &mgr) {
  const LangOptions &LO = mgr.getLangOpts();
  return LO.CPlusPlus;
}
