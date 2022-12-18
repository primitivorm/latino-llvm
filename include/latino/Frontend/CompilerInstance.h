//===-- CompilerInstance.h - Clang Compiler Instance ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_FRONTEND_COMPILERINSTANCE_H_
#define LLVM_LATINO_FRONTEND_COMPILERINSTANCE_H_

#include "latino/Basic/FileManager.h"

#include "latino/Frontend/CompilerInvocation.h"
#include "latino/Lex/ModuleLoader.h"
#include "latino/Sema/Sema.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/BuryPointer.h"
#include <cassert>
#include <list>
#include <memory>
#include <string>
#include <utility>

namespace llvm {
class Timer;
}

namespace latino {
class FrontendAction;
class Sema;
class FileManager;
class Preprocessor;

/// CompilerInstance - Helper class for managing a single instance of the Clang
/// compiler.
///
/// The CompilerInstance serves two purposes:
///  (1) It manages the various objects which are necessary to run the compiler,
///      for example the preprocessor, the target information, and the AST
///      context.
///  (2) It provides utility routines for constructing and manipulating the
///      common Clang objects.
///
/// The compiler instance generally owns the instance of all the objects that it
/// manages. However, clients can still share objects by manually setting the
/// object and retaking ownership prior to destroying the CompilerInstance.
///
/// The compiler instance is intended to simplify clients, but not to lock them
/// in to the compiler instance for everything. When possible, utility functions
/// come in two forms; a short form that reuses the CompilerInstance objects,
/// and a long form that takes explicit instances of any required objects.
class CompilerInstance : public ModuleLoader {

  /// The options used in this compiler instance.
  std::shared_ptr<CompilerInvocation> Invocation;

  /// The semantic analysis object.
  std::unique_ptr<Sema> TheSema;

  /// The frontend timer.
  std::unique_ptr<llvm::Timer> FrontendTimer;

  /// The preprocessor.
  std::shared_ptr<Preprocessor> PP;

  /// The file manager.
  llvm::IntrusiveRefCntPtr<FileManager> FileMgr;

  CompilerInstance(const CompilerInstance &) = delete;
  void operator=(const CompilerInstance &) = delete;

public:
  explicit CompilerInstance();

  ~CompilerInstance() override;

  /// @name High-Level Operations
  /// {

  /// ExecuteAction - Execute the provided action against the compiler's
  /// CompilerInvocation object.
  ///
  /// This function makes the following assumptions:
  ///
  ///  - The invocation options should be initialized. This function does not
  ///    handle the '-help' or '-version' options, clients should handle those
  ///    directly.
  ///
  ///  - The diagnostics engine should have already been created by the client.
  ///
  ///  - No other CompilerInstance state should have been initialized (this is
  ///    an unchecked error).
  ///
  ///  - Clients should have initialized any LLVM target features that may be
  ///    required.
  ///
  ///  - Clients should eventually call llvm_shutdown() upon the completion of
  ///    this routine to ensure that any managed objects are properly destroyed.
  ///
  /// Note that this routine may write output to 'stderr'.
  ///
  /// \param Act - The action to execute.
  /// \return - True on success.
  //
  // FIXME: Eliminate the llvm_shutdown requirement, that should either be part
  // of the context or else not CompilerInstance specific.
  bool ExecuteAction(FrontendAction &Act);

  /// setInvocation - Replace the current invocation.
  void setInvocation(std::shared_ptr<CompilerInvocation> Value);

  Sema &getSema() {
    assert(TheSema && "Compiler instance has no Sema object!");
    return *TheSema;
  }

  bool hasFrontendTimer() const { return (bool)FrontendTimer; }

  FrontendOptions &getFrontendOpts() { return Invocation->getFrontendOpts(); }
  const FrontendOptions &getFrontendOpts() const {
    return Invocation->getFrontendOpts();
  }

  /// Indicates whether we should (re)build the global module index.
  bool shouldBuildGlobalModuleIndex() const;

  llvm::Timer &getFrontendTimer() const {
    assert(FrontendTimer && "Compiler instance has no frontend timer!");
    return *FrontendTimer;
  }

  bool hasFileManager() const { return FileMgr != nullptr; }

  bool hasPreprocessor() const { return PP != nullptr; }

  /// Return the current preprocessor.
  Preprocessor &getPreprocessor() const {
    assert(PP && "Compiler instance has no preprocessor!");
    return *PP;
  }
};
} // namespace latino

#endif