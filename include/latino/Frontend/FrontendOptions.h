//===- FrontendOptions.h ----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_FRONTEND_FRONTENDOPTIONS_H
#define LLVM_LATINO_FRONTEND_FRONTENDOPTIONS_H

#include "latino/Basic/LangStandard.h"

#include "llvm/ADT/StringRef.h"

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace latino {
namespace frontend {

enum ActionKind {
  /// Parse ASTs and list Decl nodes.
  ASTDeclList,

  /// Parse ASTs and dump them.
  ASTDump,

  /// Parse ASTs and print them.
  ASTPrint,

  /// Parse ASTs and view them in Graphviz.
  ASTView,

  /// Dump the compiler configuration.
  DumpCompilerOptions,

  /// Dump out raw tokens.
  DumpRawTokens,

  /// Dump out preprocessed tokens.
  DumpTokens,

  /// Emit a .s file.
  EmitAssembly,

  /// Emit a .bc file.
  EmitBC,

  /// Translate input source into HTML.
  EmitHTML,

  /// Emit a .ll file.
  EmitLLVM,

  /// Generate LLVM IR, but do not emit anything.
  EmitLLVMOnly,

  /// Generate machine code, but don't emit anything.
  EmitCodeGenOnly,

  /// Emit a .o file.
  EmitObj,

  /// Parse and apply any fixits to the source.
  FixIt,

  /// Generate pre-compiled module from a module map.
  GenerateModule,

  /// Generate pre-compiled module from a C++ module interface file.
  GenerateModuleInterface,

  /// Generate pre-compiled module from a set of header files.
  GenerateHeaderModule,

  /// Generate pre-compiled header.
  GeneratePCH,

  /// Generate Interface Stub Files.
  GenerateInterfaceStubs,

  /// Only execute frontend initialization.
  InitOnly,

  /// Dump information about a module file.
  ModuleFileInfo,

  /// Load and verify that a PCH file is usable.
  VerifyPCH,

  /// Parse and perform semantic analysis.
  ParseSyntaxOnly,

  /// Run a plugin action, \see ActionName.
  PluginAction,

  /// Print the "preamble" of the input file
  PrintPreamble,

  /// -E mode.
  PrintPreprocessedInput,

  /// Expand macros but not \#includes.
  RewriteMacros,

  /// ObjC->C Rewriter.
  // RewriteObjC,

  /// Rewriter playground
  RewriteTest,

  /// Run one or more source code analyses.
  RunAnalysis,

  /// Dump template instantiations
  TemplightDump,

  /// Run migrator.
  MigrateSource,

  /// Just lex, no output.
  RunPreprocessorOnly,

  /// Print the output of the dependency directives source minimizer.
  PrintDependencyDirectivesSourceMinimizerOutput
};

}

/// The kind of a file that we've been handed as an input.
class InputKind {
private:
  Language Lang;
  unsigned Fmt : 3;
  unsigned Preprocessed : 1;

public:
  /// The input file format.
  enum Format { Source, ModuleMap, Precompiled };

  constexpr InputKind(Language L = Language::Unknown, Format F = Source,
                      bool PP = false)
      : Lang(L), Fmt(F), Preprocessed(PP) {}
};

/// An input file for the front end.
class FrontendInputFile {
  /// The file name, or "-" to read from standard input.
  std::string File;

  /// The input, if it comes from a buffer rather than a file. This object
  /// does not own the buffer, and the caller is responsible for ensuring
  /// that it outlives any users.
  const llvm::MemoryBuffer *Buffer = nullptr;

  /// The kind of input, e.g., C source, AST file, LLVM IR.
  InputKind Kind;

  /// Whether we're dealing with a 'system' input (vs. a 'user' input).
  bool IsSystem = false;

public:
  FrontendInputFile() = default;
  FrontendInputFile(llvm::StringRef File, InputKind Kind, bool IsSystem = false)
      : File(File.str()), Kind(Kind), IsSystem(IsSystem) {}
  FrontendInputFile(const llvm::MemoryBuffer *Buffer, InputKind Kind,
                    bool IsSystem = false)
      : Buffer(Buffer), Kind(Kind), IsSystem(IsSystem) {}
};

/// FrontendOptions - Options for controlling the behavior of the frontend.
class FrontendOptions {
public:
  /// Show frontend performance metrics and statistics.
  unsigned ShowStats : 1;

  /// Skip over function bodies to speed up parsing in cases you do not need
  /// them (e.g. with code completion).
  unsigned SkipFunctionBodies : 1;

  /// The input files and their types.
  llvm::SmallVector<FrontendInputFile, 0> Inputs;

  /// The frontend action to perform.
  frontend::ActionKind ProgramAction = frontend::ParseSyntaxOnly;

  FrontendOptions() {}
};

} // namespace latino

#endif