//===--- TextDiagnostics.cpp - Text Diagnostics for Paths -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the TextDiagnostics object.
//
//===----------------------------------------------------------------------===//

#include "latino/Analysis/PathDiagnostic.h"
#include "latino/Basic/SourceManager.h"
#include "latino/Basic/Version.h"
#include "latino/CrossTU/CrossTranslationUnit.h"
#include "latino/Frontend/ASTUnit.h"
#include "latino/Lex/Preprocessor.h"
#include "latino/Rewrite/Core/Rewriter.h"
#include "latino/StaticAnalyzer/Core/AnalyzerOptions.h"
#include "latino/StaticAnalyzer/Core/PathDiagnosticConsumers.h"
#include "latino/Tooling/Core/Replacement.h"
#include "latino/Tooling/Tooling.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Casting.h"

using namespace latino;
using namespace ento;
using namespace tooling;

namespace {
/// Emitsd minimal diagnostics (report message + notes) for the 'none' output
/// type to the standard error, or to to compliment many others. Emits detailed
/// diagnostics in textual format for the 'text' output type.
class TextDiagnostics : public PathDiagnosticConsumer {
  DiagnosticsEngine &DiagEng;
  const LangOptions &LO;
  const bool IncludePath = false;
  const bool ShouldEmitAsError = false;
  const bool ApplyFixIts = false;
  const bool ShouldDisplayCheckerName = false;

public:
  TextDiagnostics(DiagnosticsEngine &DiagEng, const LangOptions &LO,
                  bool ShouldIncludePath, const AnalyzerOptions &AnOpts)
      : DiagEng(DiagEng), LO(LO), IncludePath(ShouldIncludePath),
        ShouldEmitAsError(AnOpts.AnalyzerWerror),
        ApplyFixIts(AnOpts.ShouldApplyFixIts),
        ShouldDisplayCheckerName(AnOpts.ShouldDisplayCheckerNameForText) {}
  ~TextDiagnostics() override {}

  StringRef getName() const override { return "TextDiagnostics"; }

  bool supportsLogicalOpControlFlow() const override { return true; }
  bool supportsCrossFileDiagnostics() const override { return true; }

  PathGenerationScheme getGenerationScheme() const override {
    return IncludePath ? Minimal : None;
  }

  void FlushDiagnosticsImpl(std::vector<const PathDiagnostic *> &Diags,
                            FilesMade *filesMade) override {
    unsigned WarnID =
        ShouldEmitAsError
            ? DiagEng.getCustomDiagID(DiagnosticsEngine::Error, "%0")
            : DiagEng.getCustomDiagID(DiagnosticsEngine::Warning, "%0");
    unsigned NoteID = DiagEng.getCustomDiagID(DiagnosticsEngine::Note, "%0");
    SourceManager &SM = DiagEng.getSourceManager();

    Replacements Repls;
    auto reportPiece = [&](unsigned ID, FullSourceLoc Loc, StringRef String,
                           ArrayRef<SourceRange> Ranges,
                           ArrayRef<FixItHint> Fixits) {
      if (!ApplyFixIts) {
        DiagEng.Report(Loc, ID) << String << Ranges << Fixits;
        return;
      }

      DiagEng.Report(Loc, ID) << String << Ranges;
      for (const FixItHint &Hint : Fixits) {
        Replacement Repl(SM, Hint.RemoveRange, Hint.CodeToInsert);

        if (llvm::Error Err = Repls.add(Repl)) {
          llvm::errs() << "Error applying replacement " << Repl.toString()
                       << ": " << Err << "\n";
        }
      }
    };

    for (std::vector<const PathDiagnostic *>::iterator I = Diags.begin(),
         E = Diags.end();
         I != E; ++I) {
      const PathDiagnostic *PD = *I;
      std::string WarningMsg =
          (ShouldDisplayCheckerName ? " [" + PD->getCheckerName() + "]" : "")
              .str();

      reportPiece(WarnID, PD->getLocation().asLocation(),
                  (PD->getShortDescription() + WarningMsg).str(),
                  PD->path.back()->getRanges(), PD->path.back()->getFixits());

      // First, add extra notes, even if paths should not be included.
      for (const auto &Piece : PD->path) {
        if (!isa<PathDiagnosticNotePiece>(Piece.get()))
          continue;

        reportPiece(NoteID, Piece->getLocation().asLocation(),
                    Piece->getString(), Piece->getRanges(),
                    Piece->getFixits());
      }

      if (!IncludePath)
        continue;

      // Then, add the path notes if necessary.
      PathPieces FlatPath = PD->path.flatten(/*ShouldFlattenMacros=*/true);
      for (const auto &Piece : FlatPath) {
        if (isa<PathDiagnosticNotePiece>(Piece.get()))
          continue;

        reportPiece(NoteID, Piece->getLocation().asLocation(),
                    Piece->getString(), Piece->getRanges(),
                    Piece->getFixits());
      }
    }

    if (!ApplyFixIts || Repls.empty())
      return;

    Rewriter Rewrite(SM, LO);
    if (!applyAllReplacements(Repls, Rewrite)) {
      llvm::errs() << "An error occured during applying fix-it.\n";
    }

    Rewrite.overwriteChangedFiles();
  }
};
} // end anonymous namespace

void ento::createTextPathDiagnosticConsumer(
    AnalyzerOptions &AnalyzerOpts, PathDiagnosticConsumers &C,
    const std::string &Prefix, const latino::Preprocessor &PP,
    const cross_tu::CrossTranslationUnitContext &CTU) {
  C.emplace_back(new TextDiagnostics(PP.getDiagnostics(), PP.getLangOpts(),
                                     /*ShouldIncludePath*/ true, AnalyzerOpts));
}

void ento::createTextMinimalPathDiagnosticConsumer(
    AnalyzerOptions &AnalyzerOpts, PathDiagnosticConsumers &C,
    const std::string &Prefix, const latino::Preprocessor &PP,
    const cross_tu::CrossTranslationUnitContext &CTU) {
  C.emplace_back(new TextDiagnostics(PP.getDiagnostics(), PP.getLangOpts(),
                                     /*ShouldIncludePath*/ false,
                                     AnalyzerOpts));
}
