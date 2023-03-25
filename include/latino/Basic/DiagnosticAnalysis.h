//===--- DiagnosticAnalysis.h - Diagnostics for libanalysis -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_BASIC_DIAGNOSTICANALYSIS_H
#define LLVM_LATINO_BASIC_DIAGNOSTICANALYSIS_H

#include "latino/Basic/Diagnostic.h"

namespace latino {
namespace diag {
enum {
#define DIAG(ENUM, FLAGS, DEFAULT_MAPPING, DESC, GROUP, SFINAE, NOWERROR,      \
             SHOWINSYSHEADER, CATEGORY)                                        \
  ENUM,
#define ANALYSISSTART
#include "latino/Basic/DiagnosticAnalysisKinds.inc"
#undef DIAG
  NUM_BUILTIN_ANALYSIS_DIAGNOSTICS
};
} // end namespace diag
} // end namespace latino

#endif // LLVM_LATINO_BASIC_DIAGNOSTICANALYSIS_H
