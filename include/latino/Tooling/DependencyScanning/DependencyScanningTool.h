//===- DependencyScanningTool.h - clang-scan-deps service -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_TOOLING_DEPENDENCY_SCANNING_TOOL_H
#define LLVM_LATINO_TOOLING_DEPENDENCY_SCANNING_TOOL_H

#include "latino/Tooling/DependencyScanning/DependencyScanningService.h"
#include "latino/Tooling/DependencyScanning/DependencyScanningWorker.h"
#include "latino/Tooling/DependencyScanning/ModuleDepCollector.h"
#include "latino/Tooling/JSONCompilationDatabase.h"
#include "llvm/ADT/StringSet.h"
#include <string>

namespace latino{
namespace tooling{
namespace dependencies{

/// The full dependencies and module graph for a specific input.
struct FullDependencies {
  /// The name of the C++20 module this translation unit exports. This may
  /// include `:` for C++20 module partitons.
  ///
  /// If the translation unit is not a module then this will be empty.
  std::string ExportedModuleName;

  /// The context hash represents the set of compiler options that may make one
  /// version of a module incompatible with another. This includes things like
  /// language mode, predefined macros, header search paths, etc...
  ///
  /// Modules with the same name but a different \c ContextHash should be
  /// treated as separate modules for the purpose of a build.
  std::string ContextHash;

  /// A collection of absolute paths to files that this translation unit
  /// directly depends on, not including transitive dependencies.
  std::vector<std::string> FileDeps;

  /// A list of modules this translation unit directly depends on, not including
  /// transitive dependencies.
  ///
  /// This may include modules with a different context hash when it can be
  /// determined that the differences are benign for this compilation.
  std::vector<ClangModuleDep> ClangModuleDeps;

  /// A partial addtional set of command line arguments that can be used to
  /// build this translation unit.
  ///
  /// Call \c getFullAdditionalCommandLine() to get a command line suitable for
  /// appending to the original command line to pass to clang.
  std::vector<std::string> AdditionalNonPathCommandLine;

  /// Gets the full addtional command line suitable for appending to the
  /// original command line to pass to clang.
  ///
  /// \param LookupPCMPath this function is called to fill in `-fmodule-file=`
  ///                      flags and for the `-o` flag. It needs to return a
  ///                      path for where the PCM for the given module is to
  ///                      be located.
  /// \param LookupModuleDeps this fucntion is called to collect the full
  ///                         transitive set of dependencies for this
  ///                         compilation.
  std::vector<std::string> getAdditionalCommandLine(
      std::function<StringRef(ClangModuleDep)> LookupPCMPath,
      std::function<const ModuleDeps &(ClangModuleDep)> LookupModuleDeps) const;
};

struct FullDependenciesResult {
  FullDependencies FullDeps;
  std::vector<ModuleDeps> DiscoveredModules;
};

/// The high-level implementation of the dependency discovery tool that runs on
/// an individual worker thread.
class DependencyScanningTool {
public:
  /// Construct a dependency scanning tool.
  DependencyScanningTool(DependencyScanningService &Service);

  /// Print out the dependency information into a string using the dependency
  /// file format that is specified in the options (-MD is the default) and
  /// return it.
  ///
  /// \returns A \c StringError with the diagnostic output if clang errors
  /// occurred, dependency file contents otherwise.
  llvm::Expected<std::string>
  getDependencyFile(const tooling::CompilationDatabase &Compilations,
                    StringRef CWD);

  /// Collect the full module depenedency graph for the input, ignoring any
  /// modules which have already been seen.
  ///
  /// \param AlreadySeen this is used to not report modules that have previously
  ///                    been reported. Use the same `llvm::StringSet<>` for all
  ///                    calls to `getFullDependencies` for a single
  ///                    `DependencyScanningTool` for a single build. Use a
  ///                    different one for different tools, and clear it between
  ///                    builds.
  ///
  /// \returns a \c StringError with the diagnostic output if clang errors
  /// occurred, \c FullDependencies otherwise.
  llvm::Expected<FullDependenciesResult>
  getFullDependencies(const tooling::CompilationDatabase &Compilations,
                      StringRef CWD, const llvm::StringSet<> &AlreadySeen);

private:
  DependencyScanningWorker Worker;
};

} // end namespace dependencies
} // end namespace tooling
} // end namespace latino

#endif // LLVM_LATINO_TOOLING_DEPENDENCY_SCANNING_TOOL_H
