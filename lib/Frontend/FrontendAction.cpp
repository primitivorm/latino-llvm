//===--- FrontendAction.cpp -----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "latino/Frontend/FrontendAction.h"
#include "latino/Frontend/CompilerInstance.h"
#include "latino/Parse/ParseAST.h"

#include "llvm/Support/BuryPointer.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/raw_ostream.h"
#include <system_error>

using namespace latino;

FrontendAction::FrontendAction() : Instance(nullptr) {}

FrontendAction::~FrontendAction() {}

//===----------------------------------------------------------------------===//
// Utility Actions
//===----------------------------------------------------------------------===//

void ASTFrontendAction::ExecuteAction() {
  CompilerInstance &CI = getCompilerInstance();
  // if (CI.hasPreprocessor())
  //   return;

  // FIXME: Move the truncation aspect of this into Sema, we delayed this till
  // here so the source manager would be initialized.
  // if (hasCodeCompletionSupport() &&
  //     !CI.getFrontendOpts().CodeCompletionAt.FileName.empty())
  //   CI.createCodeCompletionConsumer();

  // Use a code completion consumer?
  // CodeCompleteConsumer *CompletionConsumer = nullptr;
  // if (CI.hasCodeCompletionConsumer())
  //   CompletionConsumer = &CI.getCodeCompletionConsumer();

  // if (!CI.hasSema())
  //   CI.createSema(getTranslationUnitKind(), CompletionConsumer);

  ParseAST(CI.getSema(), CI.getFrontendOpts().ShowStats,
           CI.getFrontendOpts().SkipFunctionBodies);
}

llvm::Error FrontendAction::Execute() {
  CompilerInstance &CI = getCompilerInstance();

  if (CI.hasFrontendTimer()) {
    llvm::TimeRegion Timer(CI.getFrontendTimer());
    ExecuteAction();
  } else
    ExecuteAction();

  // If we are supposed to rebuild the global module index, do so now unless
  // there were any module-build failures.
  // if (CI.shouldBuildGlobalModuleIndex() && CI.hasFileManager() &&
  //     CI.hasPreprocessor()) {
  //   llvm::StringRef Cache =
  //       CI.getPreprocessor().getHeaderSearchInfo().getModuleCachePath();
  //   if (!Cache.empty()) {
  //     if (llvm::Error Err = GlobalModuleIndex::writeIndex(
  //             CI.getFileManager(), CI.getPCHContainerReader(), Cache)) {
  //       // FIXME this drops the error on the floor, but
  //       // Index/pch-from-libclang.c seems to rely on dropping at least some
  //       of
  //       // the error conditions!
  //       consumeError(std::move(Err));
  //     }
  //   }
  // }

  return llvm::Error::success();
}