//===--- PPCallbacks.cpp - Callbacks for Preprocessor actions ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "latino/Lex/PPCallbacks.h"
#include "latino/Basic/FileManager.h"

using namespace latino;

// Out of line key method.
PPCallbacks::~PPCallbacks() = default;

void PPCallbacks::HasInclude(SourceLocation Loc, StringRef FileName,
                             bool IsAngled, Optional<FileEntryRef> File,
                             SrcMgr::CharacteristicKind FileType) {}

// Out of line key method.
PPChainedCallbacks::~PPChainedCallbacks() = default;

void PPChainedCallbacks::HasInclude(SourceLocation Loc, StringRef FileName,
                                    bool IsAngled, Optional<FileEntryRef> File,
                                    SrcMgr::CharacteristicKind FileType) {
  First->HasInclude(Loc, FileName, IsAngled, File, FileType);
  Second->HasInclude(Loc, FileName, IsAngled, File, FileType);
}

