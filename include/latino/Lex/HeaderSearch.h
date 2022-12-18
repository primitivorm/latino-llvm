//===- HeaderSearch.h - Resolve Header File Locations -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the HeaderSearch interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_LEX_HEADERSEARCH_H
#define LLVM_LATINO_LEX_HEADERSEARCH_H

#include "latino/Basic/LangOptions.h"
#include "latino/Basic/SourceLocation.h"
#include "latino/Basic/SourceManager.h"
#include "latino/Basic/TargetInfo.h"
#include "latino/Lex/DirectoryLookup.h"
#include "latino/Lex/HeaderSearchOptions.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Support/Allocator.h"
#include <cassert>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace latino {
class HeaderSearchOptions;
class TargetInfo;

/// Encapsulates the information needed to find the file referenced
/// by a \#include or \#include_next, (sub-)framework lookup, etc.
class HeaderSearch {
  FileManager &FileMgr;

  /// Header-search options used to initialize this header search.
  std::shared_ptr<HeaderSearchOptions> HSOpts;

public:
  HeaderSearch(std::shared_ptr<HeaderSearchOptions> HSOpts,
               SourceManager &SourceMgr, /*DiagnosticsEngine &Diags,*/
               const LangOptions &LangOpts, const TargetInfo *Target);

  HeaderSearch(const HeaderSearch &) = delete;
  HeaderSearch &operator=(const HeaderSearch &) = delete;

  FileManager &getFileMgr() const { return FileMgr; }
};

} // namespace latino

#endif