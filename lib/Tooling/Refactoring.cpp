//===--- Refactoring.cpp - Framework for clang refactoring tools ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  Implements tools to support refactorings.
//
//===----------------------------------------------------------------------===//

#include "latino/Tooling/Refactoring.h"
#include "latino/Basic/DiagnosticOptions.h"
#include "latino/Basic/FileManager.h"
#include "latino/Basic/SourceManager.h"
#include "latino/Format/Format.h"
#include "latino/Frontend/TextDiagnosticPrinter.h"
#include "latino/Lex/Lexer.h"
#include "latino/Rewrite/Core/Rewriter.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_os_ostream.h"

namespace latino {
namespace tooling {

RefactoringTool::RefactoringTool(
    const CompilationDatabase &Compilations, ArrayRef<std::string> SourcePaths,
    std::shared_ptr<PCHContainerOperations> PCHContainerOps)
    : ClangTool(Compilations, SourcePaths, std::move(PCHContainerOps)) {}

std::map<std::string, Replacements> &RefactoringTool::getReplacements() {
  return FileToReplaces;
}

int RefactoringTool::runAndSave(FrontendActionFactory *ActionFactory) {
  if (int Result = run(ActionFactory)) {
    return Result;
  }

  LangOptions DefaultLangOptions;
  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
  TextDiagnosticPrinter DiagnosticPrinter(llvm::errs(), &*DiagOpts);
  DiagnosticsEngine Diagnostics(
      IntrusiveRefCntPtr<DiagnosticIDs>(new DiagnosticIDs()),
      &*DiagOpts, &DiagnosticPrinter, false);
  SourceManager Sources(Diagnostics, getFiles());
  Rewriter Rewrite(Sources, DefaultLangOptions);

  if (!applyAllReplacements(Rewrite)) {
    llvm::errs() << "Skipped some replacements.\n";
  }

  return saveRewrittenFiles(Rewrite);
}

bool RefactoringTool::applyAllReplacements(Rewriter &Rewrite) {
  bool Result = true;
  for (const auto &Entry : groupReplacementsByFile(
           Rewrite.getSourceMgr().getFileManager(), FileToReplaces))
    Result = tooling::applyAllReplacements(Entry.second, Rewrite) && Result;
  return Result;
}

int RefactoringTool::saveRewrittenFiles(Rewriter &Rewrite) {
  return Rewrite.overwriteChangedFiles() ? 1 : 0;
}

bool formatAndApplyAllReplacements(
    const std::map<std::string, Replacements> &FileToReplaces,
    Rewriter &Rewrite, StringRef Style) {
  SourceManager &SM = Rewrite.getSourceMgr();
  FileManager &Files = SM.getFileManager();

  bool Result = true;
  for (const auto &FileAndReplaces : groupReplacementsByFile(
           Rewrite.getSourceMgr().getFileManager(), FileToReplaces)) {
    const std::string &FilePath = FileAndReplaces.first;
    auto &CurReplaces = FileAndReplaces.second;

    const FileEntry *Entry = nullptr;
    if (auto File = Files.getFile(FilePath))
      Entry = *File;

    FileID ID = SM.getOrCreateFileID(Entry, SrcMgr::C_User);
    StringRef Code = SM.getBufferData(ID);

    auto CurStyle = format::getStyle(Style, FilePath, "LLVM");
    if (!CurStyle) {
      llvm::errs() << llvm::toString(CurStyle.takeError()) << "\n";
      return false;
    }

    auto NewReplacements =
        format::formatReplacements(Code, CurReplaces, *CurStyle);
    if (!NewReplacements) {
      llvm::errs() << llvm::toString(NewReplacements.takeError()) << "\n";
      return false;
    }
    Result = applyAllReplacements(*NewReplacements, Rewrite) && Result;
  }
  return Result;
}

} // end namespace tooling
} // end namespace latino
