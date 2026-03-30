//===--- Parser.h - C Language Parser ---------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the Parser interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_PARSE_PARSER_H
#define LLVM_LATINO_PARSE_PARSER_H

#include "latino/Basic/Specifiers.h"
#include "latino/Lex/Token.h"
#include "latino/AST/DeclGroup.h"
// #include "latino/Sema/Sema.h"
#include "latino/Basic/TokenKinds.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/LangOptions.h"
#include "clang/AST/Expr.h"
#include "clang/Sema/Ownership.h"
#include "clang/Sema/Scope.h"
#include "latino/Sema/Sema.h"
#include "latino/Lex/Preprocessor.h"
#include "clang/Basic/OperatorPrecedence.h"

using namespace clang;

namespace latino {

/// Parser - This implements a parser for the C family of languages.  After
/// parsing units of the grammar, productions are invoked to handle whatever has
/// been read.
///
class Parser {

  Preprocessor &PP;

  /// Tok - The current token we are peeking ahead.  All parsing methods assume
  /// that this is valid.
  Token Tok;

  // PrevTokLocation - The location of the token we previously
  // consumed. This token is used for diagnostics where we expected to
  // see a token following another token (e.g., the ';' at the end of
  // a statement).
  clang::SourceLocation PrevTokLocation;

  unsigned short ParenCount = 0, BracketCount = 0, BraceCount = 0;
  unsigned short MisplacedModuleBeginCount = 0;

  /// Actions - These are the callbacks we invoke as we parse various constructs
  /// in the file.
  Sema &Actions;

  clang::DiagnosticsEngine &Diags;

  /// Whether the '>' token acts as an operator or not. This will be
  /// true except when we are parsing an expression within a C++
  /// template argument list, where the '>' closes the template
  /// argument list.
  bool GreaterThanIsOperator;

  /// ColonIsSacred - When this is false, we aggressively try to recover from
  /// code like "foo : bar" as if it were a typo for "foo :: bar".  This is not
  /// safe in case statements and a few other things.  This is managed by the
  /// ColonProtectionRAIIObject RAII object.
  bool ColonIsSacred;

  /// Tracker for '<' tokens that might have been intended to be treated as an
  /// angle bracket instead of a less-than comparison.
  ///
  /// This happens when the user intends to form a template-id, but typoes the
  /// template-name or forgets a 'template' keyword for a dependent template
  /// name.
  ///
  /// We track these locations from the point where we see a '<' with a
  /// name-like expression on its left until we see a '>' or '>>' that might
  /// match it.
  struct AngleBracketTracker {
    /// Flags used to rank candidate template names when there is more than one
    /// '<' in a scope.
    enum Priority : unsigned short {
      /// A non-dependent name that is a potential typo for a template name.
      PotentialTypo = 0x0,
      /// A dependent name that might instantiate to a template-name.
      DependentName = 0x2,

      /// A space appears before the '<' token.
      SpaceBeforeLess = 0x0,
      /// No space before the '<' token
      NoSpaceBeforeLess = 0x1,

      LLVM_MARK_AS_BITMASK_ENUM(/*LargestValue*/ DependentName)
    };

    struct Loc {
      clang::Expr *TemplateName;
      clang::SourceLocation LessLoc;
      AngleBracketTracker::Priority Priority;
      unsigned short ParenCount, BracketCount, BraceCount;

      bool isActive(Parser &P) const {
        return P.ParenCount == ParenCount && P.BracketCount == BracketCount &&
               P.BraceCount == BraceCount;
      }

      bool isActiveOrNested(Parser &P) const {
        return isActive(P) || P.ParenCount > ParenCount ||
               P.BracketCount > BracketCount || P.BraceCount > BraceCount;
      }
    };

    SmallVector<Loc, 8> Locs;

    /// Add an expression that might have been intended to be a template name.
    /// In the case of ambiguity, we arbitrarily select the innermost such
    /// expression, for example in 'foo < bar < baz', 'bar' is the current
    /// candidate. No attempt is made to track that 'foo' is also a candidate
    /// for the case where we see a second suspicious '>' token.
    void add(Parser &P, Expr *TemplateName, clang::SourceLocation LessLoc,
             Priority Prio) {
      if (!Locs.empty() && Locs.back().isActive(P)) {
        if (Locs.back().Priority <= Prio) {
          Locs.back().TemplateName = TemplateName;
          Locs.back().LessLoc = LessLoc;
          Locs.back().Priority = Prio;
        }
      } else {
        Locs.push_back({TemplateName, LessLoc, Prio, P.ParenCount,
                        P.BracketCount, P.BraceCount});
      }
    }

    /// Mark the current potential missing template location as having been
    /// handled (this happens if we pass a "corresponding" '>' or '>>' token
    /// or leave a bracket scope).
    void clear(Parser &P) {
      while (!Locs.empty() && Locs.back().isActiveOrNested(P))
        Locs.pop_back();
    }

    /// Get the current enclosing expression that might hve been intended to be
    /// a template name.
    Loc *getCurrent(Parser &P) {
      if (!Locs.empty() && Locs.back().isActive(P))
        return &Locs.back();
      return nullptr;
    }
  };

  AngleBracketTracker AngleBrackets;

  /// Whether to skip parsing of function bodies.
  ///
  /// This option can be used, for example, to speed up searches for
  /// declarations/definitions when indexing.
  bool SkipFunctionBodies;

  /// The location of the expression statement that is being parsed right now.
  /// Used to determine if an expression that is being parsed is a statement
  /// or just a regular sub-expression.
  clang::SourceLocation ExprStatementTokLoc;

  /// Flags describing a context in which we're parsing a statement.
  enum class ParsedStmtContext {
    /// This context permits declarations in language modes where declarations
    /// are not statements.
    AllowDeclarationsInC = 0x1,
    /// This context permits standalone OpenMP directives.
    AllowStandaloneOpenMPDirectives = 0x2,
    /// This context is at the top level of a GNU statement expression.
    InStmtExpr = 0x4,

    /// The context of a regular substatement.
    SubStmt = 0,
    /// The context of a compound-statement.
    Compound = AllowDeclarationsInC | AllowStandaloneOpenMPDirectives,

    LLVM_MARK_AS_BITMASK_ENUM(InStmtExpr)
  };

  /// Act on an expression statement that might be the last statement in a
  /// GNU statement expression. Checks whether we are actually at the end of
  /// a statement expression and builds a suitable expression statement.
  clang::StmtResult handleExprStmt(clang::ExprResult E, ParsedStmtContext StmtCtx);

public:
  Parser(Preprocessor &PP, Sema &Actions, bool SkipFunctionBodies);
  ~Parser();

  Preprocessor &getPreprocessor() const { return PP; }

  const clang::LangOptions &getLangOpts() const { return PP.getLangOpts(); }
  const clang::TargetInfo &getTargetInfo() const { return PP.getTargetInfo(); }

  Sema &getActions() const { return Actions; }

  const Token &getCurToken() const { return Tok; }
  Scope *getCurScope() const { return Actions.getCurScope(); }
  void incrementMSManglingNumber() const {
    return Actions.incrementMSManglingNumber();
  }

  // Type forwarding.  All of these are statically 'void*', but they may all be
  // different actual classes based on the actions in place.
  typedef OpaquePtr<DeclGroupRef> DeclGroupPtrTy;
  typedef OpaquePtr<clang::TemplateName> TemplateTy;

  /// Initialize - Warm up the parser.
  ///
  void Initialize();

  /// Parse the first top-level declaration in a translation unit.
  bool ParseFirstTopLevelDecl(DeclGroupPtrTy &Result);

  /// ParseTopLevelDecl - Parse one top-level declaration. Returns true if
  /// the EOF was encountered.
  bool ParseTopLevelDecl(DeclGroupPtrTy &Result, bool IsFirstDecl = false);
  bool ParseTopLevelDecl() {
    DeclGroupPtrTy Result;
    return ParseTopLevelDecl(Result);
  }

  /// ConsumeToken - Consume the current 'peek token' and lex the next one.
  /// This does not work with special tokens: string literals, code
  /// completion, annotation tokens and balanced tokens must be handled using
  /// the specific consume methods. Returns the location of the consumed
  /// token.
  clang::SourceLocation ConsumeToken() {
    assert(!isTokenSpecial() &&
           "Should consume special tokens with Consume*Token");
    PrevTokLocation = Tok.getLocation();
    PP.Lex(Tok);
    return PrevTokLocation;
  }

  bool TryConsumeToken(tok::TokenKind Expected) {
    if (Tok.isNot(Expected))
      return false;
    assert(!isTokenSpecial() &&
           "Should consume special tokens with Consume*Token");
    PrevTokLocation = Tok.getLocation();
    PP.Lex(Tok);
    return true;
  }

  bool TryConsumeToken(tok::TokenKind Expected, clang::SourceLocation &Loc) {
    if (!TryConsumeToken(Expected))
      return false;
    Loc = PrevTokLocation;
    return true;
  }

  /// ConsumeAnyToken - Dispatch to the right Consume* method based on the
  /// current token type.  This should only be used in cases where the type of
  /// the token really isn't known, e.g. in error recovery.
 clang::SourceLocation ConsumeAnyToken(bool ConsumeCodeCompletionTok = false) {
    if (isTokenParen())
      return ConsumeParen();
    if (isTokenBracket())
      return ConsumeBracket();
    if (isTokenBrace())
      return ConsumeBrace();
    if (isTokenStringLiteral())
      return ConsumeStringToken();
    // if (Tok.is(tok::code_completion))
    //   return ConsumeCodeCompletionTok ? ConsumeCodeCompletionToken()
    //                                   : handleUnexpectedCodeCompletionToken();
    // if (Tok.isAnnotation())
    //   return ConsumeAnnotationToken();
    return ConsumeToken();
  }

 clang::SourceLocation getEndOfPreviousToken() {
    return PP.getLocForEndOfToken(PrevTokLocation);
  }

  /// Retrieve the underscored keyword (_Nonnull, _Nullable) that corresponds
  /// to the given nullability kind.
  IdentifierInfo *getNullabilityKeyword(NullabilityKind nullability) {
    return Actions.getNullabilityKeyword(nullability);
  }

private:
  //===--------------------------------------------------------------------===//
  // Low-Level token peeking and consumption methods.
  //

  /// isTokenParen - Return true if the cur token is '(' or ')'.
  bool isTokenParen() const { return Tok.isOneOf(tok::l_paren, tok::r_paren); }
  /// isTokenBracket - Return true if the cur token is '[' or ']'.
  bool isTokenBracket() const {
    return Tok.isOneOf(tok::l_square, tok::r_square);
  }
  /// isTokenBrace - Return true if the cur token is '{' or '}'.
  bool isTokenBrace() const { return Tok.isOneOf(tok::l_brace, tok::r_brace); }
  /// isTokenStringLiteral - True if this token is a string-literal.
  bool isTokenStringLiteral() const {
    return tok::isStringLiteral(Tok.getKind());
  }
  /// isTokenSpecial - True if this token requires special consumption
  /// methods.
  bool isTokenSpecial() const {
    return isTokenStringLiteral() || isTokenParen() || isTokenBracket() ||
           isTokenBrace() /*|| Tok.is(tok::code_completion) || Tok.isAnnotation()*/;
  }

  /// Returns true if the current token is '=' or is a type of '='.
  /// For typos, give a fixit to '='
  bool isTokenEqualOrEqualTypo();

  // /// Return the current token to the token stream and make the given
  // /// token the current token.
  // void UnconsumeToken(Token &Consumed) {
  //   Token Next = Tok;
  //   PP.EnterToken(Consumed, /*IsReinject*/ true);
  //   PP.Lex(Tok);
  //   PP.EnterToken(Next, /*IsReinject*/ true);
  // }

  //clang::SourceLocation ConsumeAnnotationToken() {
  //   assert(Tok.isAnnotation() && "wrong consume method");
  //  clang::SourceLocation Loc = Tok.getLocation();
  //   PrevTokLocation = Tok.getAnnotationEndLoc();
  //   PP.Lex(Tok);
  //   return Loc;
  // }

  /// ConsumeParen - This consume method keeps the paren count up-to-date.
  ///
 clang::SourceLocation ConsumeParen() {
    assert(isTokenParen() && "wrong consume method");
    if (Tok.getKind() == tok::l_paren)
      ++ParenCount;
    else if (ParenCount) {
      AngleBrackets.clear(*this);
      --ParenCount; // Don't let unbalanced )'s drive the count negative.
    }
    PrevTokLocation = Tok.getLocation();
    PP.Lex(Tok);
    return PrevTokLocation;
  }

  /// ConsumeBracket - This consume method keeps the bracket count up-to-date.
  ///
 clang::SourceLocation ConsumeBracket() {
    assert(isTokenBracket() && "wrong consume method");
    if (Tok.getKind() == tok::l_square)
      ++BracketCount;
    else if (BracketCount) {
      AngleBrackets.clear(*this);
      --BracketCount; // Don't let unbalanced ]'s drive the count negative.
    }

    PrevTokLocation = Tok.getLocation();
    PP.Lex(Tok);
    return PrevTokLocation;
  }

  /// ConsumeBrace - This consume method keeps the brace count up-to-date.
  ///
 clang::SourceLocation ConsumeBrace() {
    assert(isTokenBrace() && "wrong consume method");
    if (Tok.getKind() == tok::l_brace)
      ++BraceCount;
    else if (BraceCount) {
      AngleBrackets.clear(*this);
      --BraceCount; // Don't let unbalanced }'s drive the count negative.
    }

    PrevTokLocation = Tok.getLocation();
    PP.Lex(Tok);
    return PrevTokLocation;
  }

  /// ConsumeStringToken - Consume the current 'peek token', lexing a new one
  /// and returning the token kind.  This method is specific to strings, as it
  /// handles string literal concatenation, as per C99 5.1.1.2, translation
  /// phase #6.
 clang::SourceLocation ConsumeStringToken() {
    assert(isTokenStringLiteral() &&
           "Should only consume string literals with this method");
    PrevTokLocation = Tok.getLocation();
    PP.Lex(Tok);
    return PrevTokLocation;
  }

  /// Determine if we're at the end of the file or at a transition
  /// between modules.
  bool isEofOrEom() {
    tok::TokenKind Kind = Tok.getKind();
    return Kind == tok::eof /*|| Kind == tok::annot_module_begin ||
           Kind == tok::annot_module_end || Kind == tok::annot_module_include*/;
  }

  /// Checks if the \p Level is valid for use in a fold expression.
  bool isFoldOperator(prec::Level Level) const;

  /// Checks if the \p Kind is a valid operator for fold expressions.
  bool isFoldOperator(tok::TokenKind Kind) const;

  /// GetLookAheadToken - This peeks ahead N tokens and returns that token
  /// without consuming any tokens.  LookAhead(0) returns 'Tok', LookAhead(1)
  /// returns the token after Tok, etc.
  ///
  /// Note that this differs from the Preprocessor's LookAhead method, because
  /// the Parser always has one token lexed that the preprocessor doesn't.
  ///
  const Token &GetLookAheadToken(unsigned N) {
    if (N == 0 || Tok.is(tok::eof))
      return Tok;
    return PP.LookAhead(N - 1);
  }

public:
  /// NextToken - This peeks ahead one token and returns it without
  /// consuming it.
  const Token &NextToken() { return PP.LookAhead(0); }
};
} // namespace latino

#endif