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

#include "latino/Basic/IdentifierTable.h"
#include "latino/Lex/ExternalPreprocessorSource.h"

#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/HeaderSearchOptions.h"
#include "clang/Basic/TargetInfo.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Allocator.h"
#include <cassert>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace latino {
  
/// The preprocessor keeps track of this information for each
/// file that is \#included.
struct HeaderFileInfo {
  /// True if this is a \#import'd or \#pragma once file.
  unsigned isImport : 1;

  /// True if this is a \#pragma once file.
  unsigned isPragmaOnce : 1;

  /// Keep track of whether this is a system header, and if so,
  /// whether it is C++ clean or not.  This can be set by the include paths or
  /// by \#pragma gcc system_header.  This is an instance of
  /// SrcMgr::CharacteristicKind.
  unsigned DirInfo : 3;

  /// Whether this header file info was supplied by an external source,
  /// and has not changed since.
  unsigned External : 1;

  /// Whether this header is part of a module.
  unsigned isModuleHeader : 1;

  /// Whether this header is part of the module that we are building.
  unsigned isCompilingModuleHeader : 1;

  /// Whether this structure is considered to already have been
  /// "resolved", meaning that it was loaded from the external source.
  unsigned Resolved : 1;

  /// Whether this is a header inside a framework that is currently
  /// being built.
  ///
  /// When a framework is being built, the headers have not yet been placed
  /// into the appropriate framework subdirectories, and therefore are
  /// provided via a header map. This bit indicates when this is one of
  /// those framework headers.
  unsigned IndexHeaderMapHeader : 1;

  /// Whether this file has been looked up as a header.
  unsigned IsValid : 1;

  /// The number of times the file has been included already.
  unsigned short NumIncludes = 0;

  /// The ID number of the controlling macro.
  ///
  /// This ID number will be non-zero when there is a controlling
  /// macro whose IdentifierInfo may not yet have been loaded from
  /// external storage.
  unsigned ControllingMacroID = 0;

  /// If this file has a \#ifndef XXX (or equivalent) guard that
  /// protects the entire contents of the file, this is the identifier
  /// for the macro that controls whether or not it has any effect.
  ///
  /// Note: Most clients should use getControllingMacro() to access
  /// the controlling macro of this header, since
  /// getControllingMacro() is able to load a controlling macro from
  /// external storage.
  const IdentifierInfo *ControllingMacro = nullptr;

  /// If this header came from a framework include, this is the name
  /// of the framework.
  StringRef Framework;

  HeaderFileInfo()
      : isImport(false), isPragmaOnce(false), DirInfo(clang::SrcMgr::C_User),
        External(false), isModuleHeader(false), isCompilingModuleHeader(false),
        Resolved(false), IndexHeaderMapHeader(false), IsValid(false)  {}

  /// Retrieve the controlling macro for this header file, if
  /// any.
  const IdentifierInfo *
  getControllingMacro(ExternalPreprocessorSource *External);

  /// Determine whether this is a non-default header file info, e.g.,
  /// it corresponds to an actual header we've included or tried to include.
  bool isNonDefault() const {
    return isImport || isPragmaOnce || NumIncludes || ControllingMacro ||
      ControllingMacroID;
  }
};

/// An external source of header file information, which may supply
/// information about header files already included.
class ExternalHeaderFileInfoSource {
public:
  virtual ~ExternalHeaderFileInfoSource();

  /// Retrieve the header file information for the given file entry.
  ///
  /// \returns Header file information for the given file entry, with the
  /// \c External bit set. If the file entry is not known, return a
  /// default-constructed \c HeaderFileInfo.
  virtual HeaderFileInfo GetHeaderFileInfo(const clang::FileEntry *FE) = 0;
};

    /// Encapsulates the information needed to find the file referenced    
/// by a \#include or \#include_next, (sub-)framework lookup, etc.
class HeaderSearch {

  /// All of the preprocessor-specific data about files that are
  /// included, indexed by the FileEntry's UID.
  mutable std::vector<HeaderFileInfo> FileInfo;

  /// Entity used to look up stored header file information.
  ExternalHeaderFileInfoSource *ExternalSource = nullptr;

    public:
  HeaderSearch(std::shared_ptr<clang::HeaderSearchOptions> HSOpts,
               clang::SourceManager &SourceMgr, clang::DiagnosticsEngine &Diags,
               const clang::LangOptions &LangOpts, const clang::TargetInfo *Target);
  HeaderSearch(const HeaderSearch &) = delete;
  HeaderSearch &operator=(const HeaderSearch &) = delete;

  /// Return the HeaderFileInfo structure for the specified FileEntry,
  /// in preparation for updating it in some way.
  HeaderFileInfo &getFileInfo(const clang::FileEntry *FE);

    /// Increment the count for the number of times the specified
  /// FileEntry has been entered.
  void IncrementIncludeCount(const clang::FileEntry *File) {
    ++getFileInfo(File).NumIncludes;
  }
};
}

#endif