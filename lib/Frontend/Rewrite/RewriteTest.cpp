//===--- RewriteTest.cpp - Rewriter playground ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This is a testbed.
//
//===----------------------------------------------------------------------===//

#include "latino/Lex/Preprocessor.h"
#include "latino/Rewrite/Core/TokenRewriter.h"
#include "latino/Rewrite/Frontend/Rewriters.h"
#include "llvm/Support/raw_ostream.h"

void latino::DoRewriteTest(Preprocessor &PP, raw_ostream *OS) {
  SourceManager &SM = PP.getSourceManager();
  const LangOptions &LangOpts = PP.getLangOpts();

  TokenRewriter Rewriter(SM.getMainFileID(), SM, LangOpts);

  // Throw <i> </i> tags around comments.
  for (TokenRewriter::token_iterator I = Rewriter.token_begin(),
       E = Rewriter.token_end(); I != E; ++I) {
    if (I->isNot(tok::comment)) continue;

    Rewriter.AddTokenBefore(I, "<i>");
    Rewriter.AddTokenAfter(I, "</i>");
  }


  // Print out the output.
  for (TokenRewriter::token_iterator I = Rewriter.token_begin(),
       E = Rewriter.token_end(); I != E; ++I)
    *OS << PP.getSpelling(*I);
}
