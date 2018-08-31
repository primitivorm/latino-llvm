//===--- RefactoringDiagnostic.h - ------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LATINO_TOOLING_REFACTORING_REFACTORINGDIAGNOSTIC_H
#define LATINO_TOOLING_REFACTORING_REFACTORINGDIAGNOSTIC_H

#include "latino/Basic/Diagnostic.h"
#include "latino/Basic/PartialDiagnostic.h"

namespace latino {
namespace diag {
enum {
#define DIAG(ENUM, FLAGS, DEFAULT_MAPPING, DESC, GROUP, SFINAE, NOWERROR,      \
             SHOWINSYSHEADER, CATEGORY)                                        \
  ENUM,
#define REFACTORINGSTART
#include "latino/Basic/DiagnosticRefactoringKinds.inc"
#undef DIAG
  NUM_BUILTIN_REFACTORING_DIAGNOSTICS
};
} // end namespace diag
} // end namespace latino

#endif // LATINO_TOOLING_REFACTORING_REFACTORINGDIAGNOSTIC_H
