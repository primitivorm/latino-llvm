//===- PreprocessorLexer.cpp - C Language Family Lexer --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements the PreprocessorLexer and Token interfaces.
//
//===----------------------------------------------------------------------===//

#include "latino/Lex/PreprocessorLexer.h"
#include "latino/Lex/Preprocessor.h"
#include "latino/Lex/Token.h"

#include "clang/Basic/SourceManager.h"
#include "clang/Lex/LexDiagnostic.h"
#include <cassert>

using namespace latino;

void PreprocessorLexer::anchor() {}

PreprocessorLexer::PreprocessorLexer(Preprocessor *pp, clang::FileID fid)
    : PP(pp), FID(fid) {
  if (pp)
    InitialNumSLocEntries = pp->getSourceManager().local_sloc_entry_size();
}