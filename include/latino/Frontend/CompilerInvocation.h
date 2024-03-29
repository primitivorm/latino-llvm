//===- CompilerInvocation.h - Compiler Invocation Helper Data ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_FRONTEND_COMPILERINVOCATION_H
#define LLVM_LATINO_FRONTEND_COMPILERINVOCATION_H

#include "latino/Basic/CodeGenOptions.h"
#include "latino/Basic/DiagnosticOptions.h"
#include "latino/Basic/FileSystemOptions.h"
#include "latino/Basic/LLVM.h"
#include "latino/Basic/LangOptions.h"
#include "latino/Basic/LangStandard.h"
#include "latino/Frontend/DependencyOutputOptions.h"
#include "latino/Frontend/FrontendOptions.h"
#include "latino/Frontend/MigratorOptions.h"
#include "latino/Frontend/PreprocessorOutputOptions.h"
#include "latino/StaticAnalyzer/Core/AnalyzerOptions.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/ArrayRef.h"
#include <memory>
#include <string>

namespace llvm {

class Triple;

namespace opt {

class ArgList;

} // namespace opt

namespace vfs {

class FileSystem;

} // namespace vfs

} // namespace llvm

namespace latino {

class DiagnosticsEngine;
class HeaderSearchOptions;
class PreprocessorOptions;
class TargetOptions;

/// Fill out Opts based on the options given in Args.
///
/// Args must have been created from the OptTable returned by
/// createCC1OptTable().
///
/// When errors are encountered, return false and, if Diags is non-null,
/// report the error(s).
bool ParseDiagnosticArgs(DiagnosticOptions &Opts, llvm::opt::ArgList &Args,
                         DiagnosticsEngine *Diags = nullptr,
                         bool DefaultDiagColor = true);

class CompilerInvocationBase {
public:
  /// Options controlling the language variant.
  std::shared_ptr<LangOptions> LangOpts;

  /// Options controlling the target.
  std::shared_ptr<TargetOptions> TargetOpts;

  /// Options controlling the diagnostic engine.
  IntrusiveRefCntPtr<DiagnosticOptions> DiagnosticOpts;

  /// Options controlling the \#include directive.
  std::shared_ptr<HeaderSearchOptions> HeaderSearchOpts;

  /// Options controlling the preprocessor (aside from \#include handling).
  std::shared_ptr<PreprocessorOptions> PreprocessorOpts;

  CompilerInvocationBase();
  CompilerInvocationBase(const CompilerInvocationBase &X);
  CompilerInvocationBase &operator=(const CompilerInvocationBase &) = delete;
  ~CompilerInvocationBase();

  LangOptions *getLangOpts() { return LangOpts.get(); }
  const LangOptions *getLangOpts() const { return LangOpts.get(); }

  TargetOptions &getTargetOpts() { return *TargetOpts.get(); }
  const TargetOptions &getTargetOpts() const { return *TargetOpts.get(); }

  DiagnosticOptions &getDiagnosticOpts() const { return *DiagnosticOpts; }

  HeaderSearchOptions &getHeaderSearchOpts() { return *HeaderSearchOpts; }

  const HeaderSearchOptions &getHeaderSearchOpts() const {
    return *HeaderSearchOpts;
  }

  std::shared_ptr<HeaderSearchOptions> getHeaderSearchOptsPtr() const {
    return HeaderSearchOpts;
  }

  std::shared_ptr<PreprocessorOptions> getPreprocessorOptsPtr() {
    return PreprocessorOpts;
  }

  PreprocessorOptions &getPreprocessorOpts() { return *PreprocessorOpts; }

  const PreprocessorOptions &getPreprocessorOpts() const {
    return *PreprocessorOpts;
  }
};

/// Helper class for holding the data necessary to invoke the compiler.
///
/// This class is designed to represent an abstract "invocation" of the
/// compiler, including data such as the include paths, the code generation
/// options, the warning flags, and so on.
class CompilerInvocation : public CompilerInvocationBase {
  /// Options controlling the static analyzer.
  AnalyzerOptionsRef AnalyzerOpts;

  MigratorOptions MigratorOpts;

  /// Options controlling IRgen and the backend.
  CodeGenOptions CodeGenOpts;

  /// Options controlling dependency output.
  DependencyOutputOptions DependencyOutputOpts;

  /// Options controlling file system operations.
  FileSystemOptions FileSystemOpts;

  /// Options controlling the frontend itself.
  FrontendOptions FrontendOpts;

  /// Options controlling preprocessed output.
  PreprocessorOutputOptions PreprocessorOutputOpts;

public:
  CompilerInvocation() : AnalyzerOpts(new AnalyzerOptions()) {}

  /// @name Utility Methods
  /// @{

  /// Create a compiler invocation from a list of input options.
  /// \returns true on success.
  ///
  /// \returns false if an error was encountered while parsing the arguments
  /// and attempts to recover and continue parsing the rest of the arguments.
  /// The recovery is best-effort and only guarantees that \p Res will end up in
  /// one of the vaild-to-access (albeit arbitrary) states.
  ///
  /// \param [out] Res - The resulting invocation.
  /// \param [in] CommandLineArgs - Array of argument strings, this must not
  /// contain "-cc1".
  static bool CreateFromArgs(CompilerInvocation &Res,
                             ArrayRef<const char *> CommandLineArgs,
                             DiagnosticsEngine &Diags,
                             const char *Argv0 = nullptr);

  /// Get the directory where the compiler headers
  /// reside, relative to the compiler binary (found by the passed in
  /// arguments).
  ///
  /// \param Argv0 - The program path (from argv[0]), for finding the builtin
  /// compiler path.
  /// \param MainAddr - The address of main (or some other function in the main
  /// executable), for finding the builtin compiler path.
  static std::string GetResourcesPath(const char *Argv0, void *MainAddr);

  /// Set language defaults for the given input language and
  /// language standard in the given LangOptions object.
  ///
  /// \param Opts - The LangOptions object to set up.
  /// \param IK - The input language.
  /// \param T - The target triple.
  /// \param PPOpts - The PreprocessorOptions affected.
  /// \param LangStd - The input language standard.
  static void setLangDefaults(LangOptions &Opts, InputKind IK,
                   const llvm::Triple &T, PreprocessorOptions &PPOpts,
                   LangStandard::Kind LangStd = LangStandard::lang_unspecified);

  /// Retrieve a module hash string that is suitable for uniquely
  /// identifying the conditions under which the module was built.
  std::string getModuleHash() const;

  using StringAllocator = llvm::function_ref<const char *(const llvm::Twine &)>;
  /// Generate a cc1-compatible command line arguments from this instance.
  ///
  /// \param [out] Args - The generated arguments. Note that the caller is
  /// responsible for inserting the path to the clang executable and "-cc1" if
  /// desired.
  /// \param SA - A function that given a Twine can allocate storage for a given
  /// command line argument and return a pointer to the newly allocated string.
  /// The returned pointer is what gets appended to Args.
  void generateCC1CommandLine(llvm::SmallVectorImpl<const char *> &Args,
                              StringAllocator SA) const;

  /// @}
  /// @name Option Subgroups
  /// @{

  AnalyzerOptionsRef getAnalyzerOpts() const { return AnalyzerOpts; }

  MigratorOptions &getMigratorOpts() { return MigratorOpts; }
  const MigratorOptions &getMigratorOpts() const { return MigratorOpts; }

  CodeGenOptions &getCodeGenOpts() { return CodeGenOpts; }
  const CodeGenOptions &getCodeGenOpts() const { return CodeGenOpts; }

  DependencyOutputOptions &getDependencyOutputOpts() {
    return DependencyOutputOpts;
  }

  const DependencyOutputOptions &getDependencyOutputOpts() const {
    return DependencyOutputOpts;
  }

  FileSystemOptions &getFileSystemOpts() { return FileSystemOpts; }

  const FileSystemOptions &getFileSystemOpts() const {
    return FileSystemOpts;
  }

  FrontendOptions &getFrontendOpts() { return FrontendOpts; }
  const FrontendOptions &getFrontendOpts() const { return FrontendOpts; }

  PreprocessorOutputOptions &getPreprocessorOutputOpts() {
    return PreprocessorOutputOpts;
  }

  const PreprocessorOutputOptions &getPreprocessorOutputOpts() const {
    return PreprocessorOutputOpts;
  }

  /// @}

private:
  /// Parse options for flags that expose marshalling information in their
  /// table-gen definition
  ///
  /// \param Args - The argument list containing the arguments to parse
  /// \param Diags - The DiagnosticsEngine associated with CreateFromArgs
  /// \returns - True if parsing was successful, false otherwise
  bool parseSimpleArgs(const llvm::opt::ArgList &Args,
                       DiagnosticsEngine &Diags);
};

IntrusiveRefCntPtr<llvm::vfs::FileSystem>
createVFSFromCompilerInvocation(const CompilerInvocation &CI,
                                DiagnosticsEngine &Diags);

IntrusiveRefCntPtr<llvm::vfs::FileSystem> createVFSFromCompilerInvocation(
    const CompilerInvocation &CI, DiagnosticsEngine &Diags,
    IntrusiveRefCntPtr<llvm::vfs::FileSystem> BaseFS);

} // namespace latino

#endif // LLVM_LATINO_FRONTEND_COMPILERINVOCATION_H
