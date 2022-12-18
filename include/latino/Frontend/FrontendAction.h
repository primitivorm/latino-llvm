//===-- FrontendAction.h - Generic Frontend Action Interface ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Defines the latino::FrontendAction interface and various convenience
/// abstract classes (latino::ASTFrontendAction, latino::PluginASTAction,
/// latino::PreprocessorFrontendAction, and latino::WrapperFrontendAction)
/// derived from it.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_FRONTEND_FRONTENDACTION_H
#define LLVM_LATINO_FRONTEND_FRONTENDACTION_H

#include "latino/Frontend/CompilerInstance.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include <memory>
#include <string>
#include <vector>

namespace latino {
class CompilerInstance;

/// Abstract base class for actions which can be performed by the frontend.
class FrontendAction {
  CompilerInstance *Instance;

protected:
  /// @name Implementation Action Interface
  /// @{

  /// Create the AST consumer object for this action, if supported.
  ///
  /// This routine is called as part of BeginSourceFile(), which will
  /// fail if the AST consumer cannot be created. This will not be called if the
  /// action has indicated that it only uses the preprocessor.
  ///
  /// \param CI - The current compiler instance, provided as a convenience, see
  /// getCompilerInstance().
  ///
  /// \param InFile - The current input file, provided as a convenience, see
  /// getCurrentFile().
  ///
  /// \return The new AST consumer, or null on failure.
  virtual std::unique_ptr<ASTConsumer>
  CreateASTConsumer(CompilerInstance &CI, llvm::StringRef InFile) = 0;

  /// Callback at the start of processing a single input.
  ///
  /// \return True on success; on failure ExecutionAction() and
  /// EndSourceFileAction() will not be called.
  virtual bool BeginSourceFileAction(CompilerInstance &CI) { return true; }

  /// Callback to run the program action, using the initialized
  /// compiler instance.
  ///
  /// This is guaranteed to only be called between BeginSourceFileAction()
  /// and EndSourceFileAction().
  virtual void ExecuteAction() = 0;

  /// @}

public:
  FrontendAction();
  virtual ~FrontendAction();

  /// @name Compiler Instance Access
  /// @{
  CompilerInstance &getCompilerInstance() const {
    assert(Instance && "Compiler instance not registered!");
    return *Instance;
  }

  void setCompilerInstance(CompilerInstance *Value) { Instance = Value; }

  /// Set the source manager's main input file, and run the action.
  llvm::Error Execute();
};

/// Abstract base class to use for AST consumer-based frontend actions.
class ASTFrontendAction : public FrontendAction {
protected:
  /// Implement the ExecuteAction interface by running Sema on
  /// the already-initialized AST consumer.
  ///
  /// This will also take care of instantiating a code completion consumer if
  /// the user requested it and the action supports it.
  void ExecuteAction() override;

public:
  ASTFrontendAction() {}
};

} // namespace latino

#endif