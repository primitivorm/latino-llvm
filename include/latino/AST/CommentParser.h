//===--- CommentParser.h - Doxygen comment parser ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the Doxygen comment parser.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_AST_COMMENTPARSER_H
#define LLVM_LATINO_AST_COMMENTPARSER_H

#include "latino/AST/Comment.h"
#include "latino/AST/CommentLexer.h"
#include "latino/AST/CommentSema.h"
#include "latino/Basic/Diagnostic.h"
#include "llvm/Support/Allocator.h"

namespace latino {
class SourceManager;

namespace comments {
class CommandTraits;

/// Doxygen comment parser.
class Parser {
  Parser(const Parser &) = delete;
  void operator=(const Parser &) = delete;

  friend class TextTokenRetokenizer;

  Lexer &L;

  Sema &S;

  /// Allocator for anything that goes into AST nodes.
  llvm::BumpPtrAllocator &Allocator;

  /// Source manager for the comment being parsed.
  const SourceManager &SourceMgr;

  DiagnosticsEngine &Diags;

  DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) {
    return Diags.Report(Loc, DiagID);
  }

  const CommandTraits &Traits;

  /// Current lookahead token.  We can safely assume that all tokens are from
  /// a single source file.
  Token Tok;

  /// A stack of additional lookahead tokens.
  SmallVector<Token, 8> MoreLATokens;

  void consumeToken() {
    if (MoreLATokens.empty())
      L.lex(Tok);
    else
      Tok = MoreLATokens.pop_back_val();
  }

  void putBack(const Token &OldTok) {
    MoreLATokens.push_back(Tok);
    Tok = OldTok;
  }

  void putBack(ArrayRef<Token> Toks) {
    if (Toks.empty())
      return;

    MoreLATokens.push_back(Tok);
    MoreLATokens.append(Toks.rbegin(), std::prev(Toks.rend()));

    Tok = Toks[0];
  }

  bool isTokBlockCommand() {
    return (Tok.is(tok::backslash_command) || Tok.is(tok::at_command)) &&
           Traits.getCommandInfo(Tok.getCommandID())->IsBlockCommand;
  }

public:
  Parser(Lexer &L, Sema &S, llvm::BumpPtrAllocator &Allocator,
         const SourceManager &SourceMgr, DiagnosticsEngine &Diags,
         const CommandTraits &Traits);

  /// Parse arguments for \\param command.
  void parseParamCommandArgs(ParamCommandComment *PC,
                             TextTokenRetokenizer &Retokenizer);

  /// Parse arguments for \\tparam command.
  void parseTParamCommandArgs(TParamCommandComment *TPC,
                              TextTokenRetokenizer &Retokenizer);

  void parseBlockCommandArgs(BlockCommandComment *BC,
                             TextTokenRetokenizer &Retokenizer,
                             unsigned NumArgs);

  BlockCommandComment *parseBlockCommand();
  InlineCommandComment *parseInlineCommand();

  HTMLStartTagComment *parseHTMLStartTag();
  HTMLEndTagComment *parseHTMLEndTag();

  BlockContentComment *parseParagraphOrBlockCommand();

  VerbatimBlockComment *parseVerbatimBlock();
  VerbatimLineComment *parseVerbatimLine();
  BlockContentComment *parseBlockContent();
  FullComment *parseFullComment();
};

} // end namespace comments
} // end namespace latino

#endif

