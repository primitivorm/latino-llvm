//===- DependencyScanningWorker.h - clang-scan-deps worker ===---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_TOOLING_DEPENDENCY_SCANNING_WORKER_H
#define LLVM_LATINO_TOOLING_DEPENDENCY_SCANNING_WORKER_H

#include "latino/Basic/DiagnosticOptions.h"
#include "latino/Basic/FileManager.h"
#include "latino/Basic/LLVM.h"
#include "latino/Frontend/PCHContainerOperations.h"
#include "latino/Lex/PreprocessorExcludedConditionalDirectiveSkipMapping.h"
#include "latino/Tooling/CompilationDatabase.h"
#include "latino/Tooling/DependencyScanning/DependencyScanningService.h"
#include "latino/Tooling/DependencyScanning/ModuleDepCollector.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FileSystem.h"
#include <string>

namespace latino {

class DependencyOutputOptions;

namespace tooling {
namespace dependencies {

class DependencyScanningWorkerFilesystem;

class DependencyConsumer {
public:
  virtual ~DependencyConsumer() {}

  virtual void handleFileDependency(const DependencyOutputOptions &Opts,
                                    StringRef Filename) = 0;

  virtual void handleModuleDependency(ModuleDeps MD) = 0;

  virtual void handleContextHash(std::string Hash) = 0;
};

/// An individual dependency scanning worker that is able to run on its own
/// thread.
///
/// The worker computes the dependencies for the input files by preprocessing
/// sources either using a fast mode where the source files are minimized, or
/// using the regular processing run.
class DependencyScanningWorker {
public:
  DependencyScanningWorker(DependencyScanningService &Service);

  /// Run the dependency scanning tool for a given clang driver invocation (as
  /// specified for the given Input in the CDB), and report the discovered
  /// dependencies to the provided consumer.
  ///
  /// \returns A \c StringError with the diagnostic output if clang errors
  /// occurred, success otherwise.
  llvm::Error computeDependencies(const std::string &Input,
                                  StringRef WorkingDirectory,
                                  const CompilationDatabase &CDB,
                                  DependencyConsumer &Consumer);

private:
  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts;
  std::shared_ptr<PCHContainerOperations> PCHContainerOps;
  std::unique_ptr<ExcludedPreprocessorDirectiveSkipMapping> PPSkipMappings;

  llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> RealFS;
  /// The file system that is used by each worker when scanning for
  /// dependencies. This filesystem persists accross multiple compiler
  /// invocations.
  llvm::IntrusiveRefCntPtr<DependencyScanningWorkerFilesystem> DepFS;
  /// The file manager that is reused accross multiple invocations by this
  /// worker. If null, the file manager will not be reused.
  llvm::IntrusiveRefCntPtr<FileManager> Files;
  ScanningOutputFormat Format;
};

} // end namespace dependencies
} // end namespace tooling
} // end namespace latino

#endif // LLVM_LATINO_TOOLING_DEPENDENCY_SCANNING_WORKER_H
