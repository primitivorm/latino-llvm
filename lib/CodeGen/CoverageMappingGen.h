//===---- CoverageMappingGen.h - Coverage mapping generation ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Instrumentation-based code coverage mapping generator
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_LIB_CODEGEN_COVERAGEMAPPINGGEN_H
#define LLVM_LATINO_LIB_CODEGEN_COVERAGEMAPPINGGEN_H

#include "latino/Basic/LLVM.h"
#include "latino/Basic/SourceLocation.h"
#include "latino/Lex/PPCallbacks.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/Support/raw_ostream.h"

namespace latino {

class LangOptions;
class SourceManager;
class FileEntry;
class Preprocessor;
class Decl;
class Stmt;

/// Stores additional source code information like skipped ranges which
/// is required by the coverage mapping generator and is obtained from
/// the preprocessor.
class CoverageSourceInfo : public PPCallbacks {
  std::vector<SourceRange> SkippedRanges;
public:
  ArrayRef<SourceRange> getSkippedRanges() const { return SkippedRanges; }

  void SourceRangeSkipped(SourceRange Range, SourceLocation EndifLoc) override;
};

namespace CodeGen {

class CodeGenModule;

/// Organizes the cross-function state that is used while generating
/// code coverage mapping data.
class CoverageMappingModuleGen {
  /// Information needed to emit a coverage record for a function.
  struct FunctionInfo {
    uint64_t NameHash;
    uint64_t FuncHash;
    std::string CoverageMapping;
    bool IsUsed;
  };

  CodeGenModule &CGM;
  CoverageSourceInfo &SourceInfo;
  llvm::SmallDenseMap<const FileEntry *, unsigned, 8> FileEntries;
  std::vector<llvm::Constant *> FunctionNames;
  std::vector<FunctionInfo> FunctionRecords;

  /// Emit a function record.
  void emitFunctionMappingRecord(const FunctionInfo &Info,
                                 uint64_t FilenamesRef);

public:
  CoverageMappingModuleGen(CodeGenModule &CGM, CoverageSourceInfo &SourceInfo)
      : CGM(CGM), SourceInfo(SourceInfo) {}

  CoverageSourceInfo &getSourceInfo() const {
    return SourceInfo;
  }

  /// Add a function's coverage mapping record to the collection of the
  /// function mapping records.
  void addFunctionMappingRecord(llvm::GlobalVariable *FunctionName,
                                StringRef FunctionNameValue,
                                uint64_t FunctionHash,
                                const std::string &CoverageMapping,
                                bool IsUsed = true);

  /// Emit the coverage mapping data for a translation unit.
  void emit();

  /// Return the coverage mapping translation unit file id
  /// for the given file.
  unsigned getFileID(const FileEntry *File);
};

/// Organizes the per-function state that is used while generating
/// code coverage mapping data.
class CoverageMappingGen {
  CoverageMappingModuleGen &CVM;
  SourceManager &SM;
  const LangOptions &LangOpts;
  llvm::DenseMap<const Stmt *, unsigned> *CounterMap;

public:
  CoverageMappingGen(CoverageMappingModuleGen &CVM, SourceManager &SM,
                     const LangOptions &LangOpts)
      : CVM(CVM), SM(SM), LangOpts(LangOpts), CounterMap(nullptr) {}

  CoverageMappingGen(CoverageMappingModuleGen &CVM, SourceManager &SM,
                     const LangOptions &LangOpts,
                     llvm::DenseMap<const Stmt *, unsigned> *CounterMap)
      : CVM(CVM), SM(SM), LangOpts(LangOpts), CounterMap(CounterMap) {}

  /// Emit the coverage mapping data which maps the regions of
  /// code to counters that will be used to find the execution
  /// counts for those regions.
  void emitCounterMapping(const Decl *D, llvm::raw_ostream &OS);

  /// Emit the coverage mapping data for an unused function.
  /// It creates mapping regions with the counter of zero.
  void emitEmptyMapping(const Decl *D, llvm::raw_ostream &OS);
};

} // end namespace CodeGen
} // end namespace latino

#endif
