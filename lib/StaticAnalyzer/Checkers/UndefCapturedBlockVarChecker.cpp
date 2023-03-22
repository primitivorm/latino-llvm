// UndefCapturedBlockVarChecker.cpp - Uninitialized captured vars -*- C++ -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This checker detects blocks that capture uninitialized values.
//
//===----------------------------------------------------------------------===//

#include "latino/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "latino/AST/Attr.h"
#include "latino/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "latino/StaticAnalyzer/Core/Checker.h"
#include "latino/StaticAnalyzer/Core/CheckerManager.h"
#include "latino/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "latino/StaticAnalyzer/Core/PathSensitive/ExprEngine.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/raw_ostream.h"

using namespace latino;
using namespace ento;

namespace {
class UndefCapturedBlockVarChecker
  : public Checker< check::PostStmt<BlockExpr> > {
  mutable std::unique_ptr<BugType> BT;

public:
  void checkPostStmt(const BlockExpr *BE, CheckerContext &C) const;
};
} // end anonymous namespace

static const DeclRefExpr *FindBlockDeclRefExpr(const Stmt *S,
                                               const VarDecl *VD) {
  if (const DeclRefExpr *BR = dyn_cast<DeclRefExpr>(S))
    if (BR->getDecl() == VD)
      return BR;

  for (const Stmt *Child : S->children())
    if (Child)
      if (const DeclRefExpr *BR = FindBlockDeclRefExpr(Child, VD))
        return BR;

  return nullptr;
}

void
UndefCapturedBlockVarChecker::checkPostStmt(const BlockExpr *BE,
                                            CheckerContext &C) const {
  if (!BE->getBlockDecl()->hasCaptures())
    return;

  ProgramStateRef state = C.getState();
  auto *R = cast<BlockDataRegion>(C.getSVal(BE).getAsRegion());

  BlockDataRegion::referenced_vars_iterator I = R->referenced_vars_begin(),
                                            E = R->referenced_vars_end();

  for (; I != E; ++I) {
    // This VarRegion is the region associated with the block; we need
    // the one associated with the encompassing context.
    const VarRegion *VR = I.getCapturedRegion();
    const VarDecl *VD = VR->getDecl();

    if (VD->hasAttr<BlocksAttr>() || !VD->hasLocalStorage())
      continue;

    // Get the VarRegion associated with VD in the local stack frame.
    if (Optional<UndefinedVal> V =
          state->getSVal(I.getOriginalRegion()).getAs<UndefinedVal>()) {
      if (ExplodedNode *N = C.generateErrorNode()) {
        if (!BT)
          BT.reset(
              new BuiltinBug(this, "uninitialized variable captured by block"));

        // Generate a bug report.
        SmallString<128> buf;
        llvm::raw_svector_ostream os(buf);

        os << "Variable '" << VD->getName()
           << "' is uninitialized when captured by block";

        auto R = std::make_unique<PathSensitiveBugReport>(*BT, os.str(), N);
        if (const Expr *Ex = FindBlockDeclRefExpr(BE->getBody(), VD))
          R->addRange(Ex->getSourceRange());
        R->addVisitor(std::make_unique<FindLastStoreBRVisitor>(
            *V, VR, /*EnableNullFPSuppression*/ false,
            bugreporter::TrackingKind::Thorough));
        R->disablePathPruning();
        // need location of block
        C.emitReport(std::move(R));
      }
    }
  }
}

void ento::registerUndefCapturedBlockVarChecker(CheckerManager &mgr) {
  mgr.registerChecker<UndefCapturedBlockVarChecker>();
}

bool ento::shouldRegisterUndefCapturedBlockVarChecker(const CheckerManager &mgr) {
  return true;
}