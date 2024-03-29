//== RetainCountDiagnostics.h - Checks for leaks and other issues -*- C++ -*--//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines diagnostics for RetainCountChecker, which implements
//  a reference count checker for Core Foundation and Cocoa on (Mac OS X).
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_LIB_STATICANALYZER_CHECKERS_RETAINCOUNTCHECKER_DIAGNOSTICS_H
#define LLVM_LATINO_LIB_STATICANALYZER_CHECKERS_RETAINCOUNTCHECKER_DIAGNOSTICS_H

#include "latino/Analysis/PathDiagnostic.h"
// #include "latino/Analysis/RetainSummaryManager.h"
#include "latino/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "latino/StaticAnalyzer/Core/BugReporter/BugReporterVisitors.h"
#include "latino/StaticAnalyzer/Core/PathSensitive/CallEvent.h"

namespace latino {
namespace ento {
namespace retaincountchecker {

class RefCountBug : public BugType {
public:
  enum RefCountBugKind {
    UseAfterRelease,
    ReleaseNotOwned,
    DeallocNotOwned,
    FreeNotOwned,
    OverAutorelease,
    ReturnNotOwnedForOwned,
    LeakWithinFunction,
    LeakAtReturn,
  };
  RefCountBug(CheckerNameRef Checker, RefCountBugKind BT);
  StringRef getDescription() const;

  RefCountBugKind getBugType() const { return BT; }

private:
  RefCountBugKind BT;
  static StringRef bugTypeToName(RefCountBugKind BT);
};

class RefCountReport : public PathSensitiveBugReport {
protected:
  SymbolRef Sym;
  bool isLeak = false;

public:
  RefCountReport(const RefCountBug &D, const LangOptions &LOpts,
              ExplodedNode *n, SymbolRef sym,
              bool isLeak=false);

  RefCountReport(const RefCountBug &D, const LangOptions &LOpts,
              ExplodedNode *n, SymbolRef sym,
              StringRef endText);

  ArrayRef<SourceRange> getRanges() const override {
    if (!isLeak)
      return PathSensitiveBugReport::getRanges();
    return {};
  }
};

class RefLeakReport : public RefCountReport {
  const MemRegion* AllocBinding;
  const Stmt *AllocStmt;
  PathDiagnosticLocation Location;

  // Finds the function declaration where a leak warning for the parameter
  // 'sym' should be raised.
  void deriveParamLocation(CheckerContext &Ctx, SymbolRef sym);
  // Finds the location where a leak warning for 'sym' should be raised.
  void deriveAllocLocation(CheckerContext &Ctx, SymbolRef sym);
  // Produces description of a leak warning which is printed on the console.
  void createDescription(CheckerContext &Ctx);

public:
  RefLeakReport(const RefCountBug &D, const LangOptions &LOpts, ExplodedNode *n,
                SymbolRef sym, CheckerContext &Ctx);
  PathDiagnosticLocation getLocation() const override {
    assert(Location.isValid());
    return Location;
  }

  PathDiagnosticLocation getEndOfPath() const {
    return PathSensitiveBugReport::getLocation();
  }
};

} // end namespace retaincountchecker
} // end namespace ento
} // end namespace latino

#endif
