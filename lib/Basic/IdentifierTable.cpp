//===- IdentifierTable.cpp - Hash table for identifier lookup -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the IdentifierInfo, IdentifierVisitor, and
// IdentifierTable interfaces.
//
//===----------------------------------------------------------------------===//

#include "clang/Basic/CharInfo.h"
#include "clang/Basic/OperatorKinds.h"
#include "clang/Basic/Specifiers.h"
#include "clang/Basic/TargetBuiltins.h"

#include "latino/Basic/IdentifierTable.h"
#include "latino/Basic/LangOptions.h"
#include "latino/Basic/TokenKinds.h"

#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/ADT/FoldingSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>

using namespace latino;

//===----------------------------------------------------------------------===//
// IdentifierTable Implementation
//===----------------------------------------------------------------------===//

IdentifierTable::IdentifierTable(IdentifierInfoLookup *ExternalLookup)
    : HashTable(8192), // Start with space for 8K identifiers.
      ExternalLookup(ExternalLookup) {}

IdentifierTable::IdentifierTable(const LangOptions &LangOpts,
                                 IdentifierInfoLookup *ExternalLookup)
    : IdentifierTable(ExternalLookup) {
  // Populate the identifier table with info about keywords for the current
  // language.
  AddKeywords(LangOpts);
}

/// AddKeyword - This method is used to associate a token ID with specific
/// identifiers because they are language keywords.  This causes the lexer to
/// automatically map matching identifiers to specialized token codes.
static void AddKeyword(StringRef Keyword, tok::TokenKind TokenCode,
                       unsigned Flags, const LangOptions &LangOpts,
                       IdentifierTable &Table) {
  IdentifierInfo &Info = Table.get(Keyword, TokenCode);
}

/// AddKeywords - Add all keywords to the symbol table.
///
void IdentifierTable::AddKeywords(const LangOptions &LangOpts) {
// Add keywords and tokens for the current language.
#define KEWYWORD(NAME, FLAGS)                                                  \
  AddKeyword(StringRef(#NAME), tok::tk_##NAME, FLAGS, LangOpts, *this);
#include "latino/Basic/TokenKinds.def"

  // Add the 'importar' contextual keyword
  get("importar").setModulesImport(true);
}
