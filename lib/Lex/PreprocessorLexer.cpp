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
#include "latino/Basic/SourceManager.h"
#include "latino/Lex/LexDiagnostic.h"
#include "latino/Lex/Preprocessor.h"
#include "latino/Lex/Token.h"
#include <cassert>

using namespace latino;

void PreprocessorLexer::anchor() {}

PreprocessorLexer::PreprocessorLexer(Preprocessor *pp, FileID fid)
    : PP(pp), FID(fid) {
  if (pp)
    InitialNumSLocEntries = pp->getSourceManager().local_sloc_entry_size();
}

/// After the preprocessor has parsed a \#include, lex and
/// (potentially) macro expand the filename.
void PreprocessorLexer::LexIncludeFilename(Token &FilenameTok) {
  assert(ParsingFilename == false && "reentered LexIncludeFilename");

  // We are now parsing a filename!
  ParsingFilename = true;

  // Lex the filename.
  if (LexingRawMode)
    IndirectLex(FilenameTok);
  else
    PP->Lex(FilenameTok);

  // We should have obtained the filename now.
  ParsingFilename = false;
}

/// getFileEntry - Return the FileEntry corresponding to this FileID.  Like
/// getFileID(), this only works for lexers with attached preprocessors.
const FileEntry *PreprocessorLexer::getFileEntry() const {
  return PP->getSourceManager().getFileEntryForID(getFileID());
}
