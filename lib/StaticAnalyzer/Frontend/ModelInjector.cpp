//===-- ModelInjector.cpp ---------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ModelInjector.h"
#include "latino/AST/Decl.h"
#include "latino/Basic/IdentifierTable.h"
#include "latino/Basic/LangStandard.h"
#include "latino/Basic/Stack.h"
#include "latino/AST/DeclObjC.h"
#include "latino/Frontend/ASTUnit.h"
#include "latino/Frontend/CompilerInstance.h"
#include "latino/Frontend/FrontendAction.h"
#include "latino/Lex/Preprocessor.h"
#include "latino/Serialization/ASTReader.h"
#include "latino/StaticAnalyzer/Frontend/FrontendActions.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/CrashRecoveryContext.h"
#include "llvm/Support/FileSystem.h"
#include <utility>

using namespace latino;
using namespace ento;

ModelInjector::ModelInjector(CompilerInstance &CI) : CI(CI) {}

Stmt *ModelInjector::getBody(const FunctionDecl *D) {
  onBodySynthesis(D);
  return Bodies[D->getName()];
}

Stmt *ModelInjector::getBody(const ObjCMethodDecl *D) {
  onBodySynthesis(D);
  return Bodies[D->getName()];
}

void ModelInjector::onBodySynthesis(const NamedDecl *D) {

  // FIXME: what about overloads? Declarations can be used as keys but what
  // about file name index? Mangled names may not be suitable for that either.
  if (Bodies.count(D->getName()) != 0)
    return;

  SourceManager &SM = CI.getSourceManager();
  FileID mainFileID = SM.getMainFileID();

  AnalyzerOptionsRef analyzerOpts = CI.getAnalyzerOpts();
  llvm::StringRef modelPath = analyzerOpts->ModelPath;

  llvm::SmallString<128> fileName;

  if (!modelPath.empty())
    fileName =
        llvm::StringRef(modelPath.str() + "/" + D->getName().str() + ".model");
  else
    fileName = llvm::StringRef(D->getName().str() + ".model");

  if (!llvm::sys::fs::exists(fileName.str())) {
    Bodies[D->getName()] = nullptr;
    return;
  }

  auto Invocation = std::make_shared<CompilerInvocation>(CI.getInvocation());

  FrontendOptions &FrontendOpts = Invocation->getFrontendOpts();
  InputKind IK = Language::CXX; // FIXME
  FrontendOpts.Inputs.clear();
  FrontendOpts.Inputs.emplace_back(fileName, IK);
  FrontendOpts.DisableFree = true;

  Invocation->getDiagnosticOpts().VerifyDiagnostics = 0;

  // Modules are parsed by a separate CompilerInstance, so this code mimics that
  // behavior for models
  CompilerInstance Instance(CI.getPCHContainerOperations());
  Instance.setInvocation(std::move(Invocation));
  Instance.createDiagnostics(
      new ForwardingDiagnosticConsumer(CI.getDiagnosticClient()),
      /*ShouldOwnClient=*/true);

  Instance.getDiagnostics().setSourceManager(&SM);

  // The instance wants to take ownership, however DisableFree frontend option
  // is set to true to avoid double free issues
  Instance.setFileManager(&CI.getFileManager());
  Instance.setSourceManager(&SM);
  Instance.setPreprocessor(CI.getPreprocessorPtr());
  Instance.setASTContext(&CI.getASTContext());

  Instance.getPreprocessor().InitializeForModelFile();

  ParseModelFileAction parseModelFile(Bodies);

  llvm::CrashRecoveryContext CRC;

  CRC.RunSafelyOnThread([&]() { Instance.ExecuteAction(parseModelFile); },
                        DesiredStackSize);

  Instance.getPreprocessor().FinalizeForModelFile();

  Instance.resetAndLeakSourceManager();
  Instance.resetAndLeakFileManager();
  Instance.resetAndLeakPreprocessor();

  // The preprocessor enters to the main file id when parsing is started, so
  // the main file id is changed to the model file during parsing and it needs
  // to be reset to the former main file id after parsing of the model file
  // is done.
  SM.setMainFileID(mainFileID);
}
