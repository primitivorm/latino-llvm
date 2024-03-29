//===--- PathDiagnosticConsumers.h - Path Diagnostic Clients ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the interface to create different path diagostic clients.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_STATICANALYZER_CORE_PATHDIAGNOSTICCONSUMERS_H
#define LLVM_LATINO_STATICANALYZER_CORE_PATHDIAGNOSTICCONSUMERS_H

#include <string>
#include <vector>

namespace latino {

class AnalyzerOptions;
class Preprocessor;
namespace cross_tu {
class CrossTranslationUnitContext;
}

namespace ento {

class PathDiagnosticConsumer;
typedef std::vector<PathDiagnosticConsumer*> PathDiagnosticConsumers;

#define ANALYSIS_DIAGNOSTICS(NAME, CMDFLAG, DESC, CREATEFN)                    \
  void CREATEFN(AnalyzerOptions &AnalyzerOpts, PathDiagnosticConsumers &C,     \
                const std::string &Prefix, const Preprocessor &PP,             \
                const cross_tu::CrossTranslationUnitContext &CTU);
#include "latino/StaticAnalyzer/Core/Analyses.def"

} // end 'ento' namespace
} // end 'clang' namespace

#endif
