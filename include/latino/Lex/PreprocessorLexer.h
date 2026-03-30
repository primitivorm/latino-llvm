//===- PreprocessorLexer.h - C Language Family Lexer ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Defines the PreprocessorLexer interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_LEX_PREPROCESSORLEXER_H
#define LLVM_LATINO_LEX_PREPROCESSORLEXER_H

#include "latino/Lex/MultipleIncludeOpt.h"
#include "latino/Lex/Token.h"
#include "clang/Basic/SourceLocation.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include <cassert>

namespace latino {
class PreprocessorLexer {
  virtual void anchor();

protected:
  friend class Preprocessor;

  // Preprocessor object controlling lexing.
  Preprocessor *PP = nullptr;

  /// The SourceManager FileID corresponding to the file being lexed.
  const clang::FileID FID;
  
  /// Number of SLocEntries before lexing the file.
  unsigned InitialNumSLocEntries = 0;

  PreprocessorLexer() : FID() {}
  PreprocessorLexer(Preprocessor *pp, clang::FileID fid);
  virtual ~PreprocessorLexer() = default;
};
} // namespace latino

#endif