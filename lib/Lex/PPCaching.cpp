//===--- PPCaching.cpp - Handle caching lexed tokens ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements pieces of the Preprocessor interface that manage the
// caching of lexed tokens.
//
//===----------------------------------------------------------------------===//

#include "latino/Lex/Token.h"
#include "latino/Lex/Preprocessor.h"
#include <cassert>

using namespace latino;

void Preprocessor::EnterCachingLexMode() {
  // The caching layer sits on top of all the other lexers, so it's incorrect
  // to cache tokens while inside a nested lex action. The cached tokens would
  // be retained after returning to the enclosing lex action and, at best,
  // would appear at the wrong position in the token stream.
  assert(LexLevel == 0 &&
         "entered caching lex mode while lexing something else");

  if (InCachingLexMode()) {
    assert(CurLexerKind == CLK_CachingLexer && "Unexpected lexer kind");
    return;
  }

  EnterCachingLexModeUnchecked();
}

void Preprocessor::EnterCachingLexModeUnchecked() {
  assert(CurLexerKind != CLK_CachingLexer && "already in caching lex mode");
  PushIncludeMacroStack();
  CurLexerKind = CLK_CachingLexer;
}

const Token &Preprocessor::PeekAhead(unsigned N) {
  assert(CachedLexPos + N > CachedTokens.size() && "Confused caching.");
  ExitCachingLexMode();
  for (size_t C = CachedLexPos + N - CachedTokens.size(); C > 0; --C) {
    CachedTokens.push_back(Token());
    Lex(CachedTokens.back());
  }
  EnterCachingLexMode();
  return CachedTokens.back();
}

