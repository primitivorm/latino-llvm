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

#include "latino/Basic/BitmaskEnum.h"
#include "latino/Basic/OperatorPrecedence.h"
#include "latino/Lex/Preprocessor.h"
#include "latino/Sema/DeclSpec.h"
#include "latino/Sema/Sema.h"

#include "llvm/ADT/SmallVector.h"
// #include "llvm/Frontend/OpenMP/OMPContext.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/SaveAndRestore.h"
#include <memory>
#include <stack>

namespace latino {
class Scope;
class Parser;
// class ParsingDeclRAIIObject;
class ParsingDeclSpec;
class ParsingDeclarator;
class ParsingFieldDeclarator;
class ColonProtectionRAIIObject;
class BalancedDelimiterTracker;
class DeclGroupRef;
class Parser;

/// Parser - This implements a parser for the C family of languages.  After
/// parsing units of the grammar, productions are invoked to handle whatever has
/// been read.
///
class Parser {
  friend class ColonProtectionRAIIObject;
  friend class ParenBraceBracketBalancer;
  friend class BalancedDelimiterTracker;

  Preprocessor &PP;

  /// Tok - The current token we are peeking ahead.  All parsing methods assume
  /// that this is valid.
  Token Tok;

  // PrevTokLocation - The location of the token we previously
  // consumed. This token is used for diagnostics where we expected to
  // see a token following another token (e.g., the ';' at the end of
  // a statement).
  SourceLocation PrevTokLocation;

  unsigned short ParenCount = 0, BracketCount = 0, BraceCount = 0;

  /// Actions - These are the callbacks we invoke as we parse various constructs
  /// in the file.
  Sema &Actions;

  DiagnosticsEngine &Diags;

  // DiagnosticsEngine &Diags;

  /// ScopeCache - Cache scopes to reduce malloc traffic.
  enum { ScopeCacheSize = 16 };
  unsigned NumCachedScopes;
  Scope *ScopeCache[ScopeCacheSize];

  // C++2a contextual keywords.
  mutable IdentifierInfo *Ident_import;
  mutable IdentifierInfo *Ident_module;

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

  /// Identifiers which have been declared within a tentative parse.
  SmallVector<IdentifierInfo *, 8> TentativelyDeclaredIdentifiers;

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
    enum Priority {
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
      SourceLocation LessLoc;
      AngleBracketTracker::Priority Priority;
      unsigned short ParenCount, BracketCount, BraceCount;

      bool isActive(Parser &P) const {
        return P.ParenCount == ParenCount && P.BracketCount == BracketCount &&
               P.BracketCount == BracketCount;
      }

      bool isActiveOrNested(Parser &P) const {
        return isActive(P) || P.ParenCount > ParenCount ||
               P.BracketCount > BracketCount || P.BraceCount > BraceCount;
      }
    };

    llvm::SmallVector<Loc, 8> Locs;

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
  /// Used to determine if an expression that is being parsed is a statement or
  /// just a regular sub-expression.
  SourceLocation ExprStatementTokLoc;

  /// Flags describing a context in which we're parsing a statement.
  enum /*class*/ ParsedStmtContext {
    /// This context permits declarations in language modes where declarations
    /// are not statements.
    AllowDeclarationsInC = 0x1,
    /// This context permits standalone OpenMP directives.
    // AllowStandaloneOpenMPDirectives = 0x2,
    /// This context is at the top level of a GNU statement expression.
    InStmtExpr = 0x4,

    /// The context of a regular substatement.
    SubStmt = 0,
    /// The context of a compound-statement.
    Compound = AllowDeclarationsInC /*| AllowStandaloneOpenMPDirectives*/,

    LLVM_MARK_AS_BITMASK_ENUM(InStmtExpr)
  };

  /// Act on an expression statement that might be the last statement in a
  /// GNU statement expression. Checks whether we are actually at the end of
  /// a statement expression and builds a suitable expression statement.
  StmtResult handleExprStmt(ExprResult E, ParsedStmtContext StmtCtx);

public:
  Parser(Preprocessor &PP, Sema &Actions, bool SkipFunctionBodies);
  // ~Parser() override;

  const LangOptions &getLangOpts() const { return PP.getLangOpts(); }
  const TargetInfo &getTargetInfo() const { return PP.getTargetInfo(); }
  Preprocessor &getPreprocessor() const { return PP; }
  Sema &getActions() const { return Actions; }

  const Token &getCurToken() const { return Tok; }
  Scope *getCurScope() const { return Actions.getCurScope(); }
  /*void incrementMSManglingNumber() const {
    return Actions.incrementMSManglingNumber();
  }*/

  // Type forwarding.  All of these are statically 'void*', but they may all be
  // different actual classes based on the actions in place.
  typedef OpaquePtr<DeclGroupRef> DeclGroupPtrTy;

  // typedef Sema::FullExprArg FullExprArg;

  // Parsing methods.

  /// Initialize - Warm up the parser.
  ///
  void Initialilze();

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
  /// This does not work with special tokens: string literals, code completion,
  /// annotation tokens and balanced tokens must be handled using the specific
  /// consume methods.
  /// Returns the location of the consumed token.
  SourceLocation ConsumeToken() {
    assert(!isTokenSpecial() &&
           "Shoul consume special tokens with Consume*Token");
    PrevTokLocation = Tok.getLocation();
    PP.Lex(Tok);
    return PrevTokLocation;
  }

  bool TryConsumeToken(tok::TokenKind Expected) {
    if (Tok.isNot(Expected))
      return false;
    assert(!isTokenSpecial() &&
           "Shoul consume special tokens with Consume*Token");
    PrevTokLocation = Tok.getLocation();
    PP.Lex(Tok);
    return true;
  }

  bool TryConsumeToken(tok::TokenKind Expected, SourceLocation &Loc) {
    if (!TryConsumeToken(Expected))
      return false;
    Loc = PrevTokLocation;
    return true;
  }

  /// ConsumeAnyToken - Dispatch to the right Consume* method based on the
  /// current token type.  This should only be used in cases where the type of
  /// the token really isn't known, e.g. in error recovery.
  SourceLocation ConsumeAnyToken(bool ConsumeCodeCompletionTok = false) {
    if (isTokenParen())
      return ConsumeParen();
    if (isTokenBracket())
      return ConsumeBracket();
    if (isTokenBrace())
      return ConsumeBrace();
    if (isTokenStringLiteral())
      return ConsumeStringToken();
    /*if (Tok.is(tok::code_completion))
      return ConsumeCodeCompletionTok ? ConsumeCodeCompletionToken()
                                      : handleUnexpectedCodeCompletionToken();*/
    if (Tok.isAnnotation())
      return ConsumeAnnotationToken();
    return ConsumeToken();
  }

  /*SourceLocation getEndOfPreviousToken() {
    return PP.getLocForEndOfToken(PrevTokLocation);
  }*/

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

  /// isTokenSpecial - True if this token requires special consumption methods.
  bool isTokenSpecial() const {
    return isTokenStringLiteral() || isTokenParen() || isTokenBracket() ||
           isTokenBrace() /*||
           Tok.is(tok::code_completion) || Tok.isAnnotation()*/
        ;
  }

  /// Returns true if the current token is '=' or is a type of '='.
  /// For typos, give a fixit to '='
  bool isTokenEqualOrEqualTypo();

  /// Return the current token to the token stream and make the given
  /// token the current token.
  void UnconsumeToken(Token &Consumed) {
    Token Next = Tok;
    PP.EnterToken(Consumed, /*IsReinject*/ true);
    PP.Lex(Tok);
    PP.EnterToken(Next, /*IsReinject*/ false);
  }

  SourceLocation ConsumeAnnotationToken() {
    assert(Tok.isAnnotation() && "wrong consume method");
    SourceLocation Loc = Tok.getLocation();
    PrevTokLocation = Tok.getAnnotationEndLoc();
    PP.Lex(Tok);
    return Loc;
  }

  /// ConsumeParen - This consume method keeps the paren count up-to-date.
  ///
  SourceLocation ConsumeParen() {
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
  SourceLocation ConsumeBracket() {
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
  SourceLocation ConsumeBrace() {
    assert(isTokenBrace() && "wrong consume method");
    if (Tok.getKind() == tok::l_brace)
      ++BracketCount;
    else if (BracketCount) {
      AngleBrackets.clear(*this);
      --BracketCount; // Don't let unbalanced }'s drive the count negative.
    }
    PrevTokLocation = Tok.getLocation();
    PP.Lex(Tok);
    return PrevTokLocation;
  }

  /// ConsumeStringToken - Consume the current 'peek token', lexing a new one
  /// and returning the token kind.  This method is specific to strings, as it
  /// handles string literal concatenation, as per C99 5.1.1.2, translation
  /// phase #6.
  SourceLocation ConsumeStringToken() {
    assert(isTokenStringLiteral() &&
           "Should only consume string literals with this method");
    PrevTokLocation = Tok.getLocation();
    PP.Lex(Tok);
    return PrevTokLocation;
  }

  /// Consume the current code-completion token.
  ///
  /// This routine can be called to consume the code-completion token and
  /// continue processing in special cases where \c cutOffParsing() isn't
  /// desired, such as token caching or completion with lookahead.
  SourceLocation ConsumeCodeCompletionToken() {
    assert(Tok.is(tok::code_completion));
    PrevTokLocation = Tok.getLocation();
    PP.Lex(Tok);
    return PrevTokLocation;
  }

  ///\ brief When we are consuming a code-completion token without having
  /// matched specific position in the grammar, provide code-completion results
  /// based on context.
  ///
  /// \returns the source location of the code-completion token.
  SourceLocation handleUnexpectedCodeCompletionToken();

  /// Abruptly cut off parsing; mainly used when we have reached the
  /// code-completion point.
  void cutOffParsing() {
    /*if (PP.isCodeCompletionEnabled())
      PP.setCodeCompletionReached();*/
    // Cut off parsing by acting as if we reached the end-of-file.
    Tok.setKind(tok::eof);
  }

  /// Determine if we're at the end of the file or at a transition
  /// between modules.
  bool isEofOrEom() {
    tok::TokenKind Kind = Tok.getKind();
    return Kind == tok::eof || Kind == tok::annot_module_begin ||
           Kind == tok::annot_module_end || Kind == tok::annot_module_include;
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

  /// getTypeAnnotation - Read a parsed type out of an annotation token.
  static TypeResult getTypeAnnotation(const Token &Tok) {
    if (!Tok.getAnnotationValue())
      return TypeError();
    return ParsedType::getFromOpaquePtr(Tok.getAnnotationValue());
  }

private:
  static void setTypeAnnotation(Token &Tok, TypeResult T) {
    assert((T.isInvalid() || T.get()) &&
           "produced a valid-but-null type annotation?");
    Tok.setAnnotationValue(T.isInvalid() ? nullptr : T.get().getAsOpaquePtr());
  }

  static NamedDecl *getNonTypeAnnotation(const Token &Tok) {
    return static_cast<NamedDecl *>(Tok.getAnnotationValue());
  }

  static void setNonTypeAnnotation(Token &Tok, NamedDecl *ND) {
    Tok.setAnnotationValue(ND);
  }

  static IdentifierInfo *getIdentifierAnnotation(const Token &Tok) {
    return static_cast<IdentifierInfo *>(Tok.getAnnotationValue());
  }

  static void setIdentifierAnnotation(Token &Tok, IdentifierInfo *ND) {
    Tok.setAnnotationValue(ND);
  }

  /// Read an already-translated primary expression out of an annotation
  /// token.
  // static ExprResult getExprAnnotation(const Token &Tok) {
  //   return ExprResult::getFromOpaquePointer(Tok.getAnnotationValue());
  // }

  /// Set the primary expression corresponding to the given annotation
  /// token.
  static void setExprAnnotation(Token &Tok, ExprResult ER) {
    Tok.setAnnotationValue(ER.getAsOpaquePointer());
  }

public:
  // If NeedType is true, then TryAnnotateTypeOrScopeToken will try harder to
  // find a type name by attempting typo correction.
  bool TryAnnotateTypeOrScopeToken();
  bool TryAnnotateTypeOrScopeTokenAfterScopeSpec(CXXScopeSpec &SS,
                                                 bool IsNewScope);
  bool TryAnnotateCXXScopeToken(bool EnteringContext = false);

  // bool MightBeCXXScopeToken() {
  //   return Tok.is(tok::identifier) || Tok.is(tok::coloncolon) ||
  //          (Tok.is(tok::annot_template_id) &&
  //           NextToken().is(tok::coloncolon)) ||
  //          Tok.is(tok::kw_decltype) || Tok.is(tok::kw___super);
  // }
  // bool TryAnnotateOptionalCXXScopeToken(bool EnteringContext = false) {
  //   return MightBeCXXScopeToken() &&
  //   TryAnnotateCXXScopeToken(EnteringContext);
  // }

private:
  enum AnnotatedNameKind {
    /// Annotation has failed and emitted an error.
    ANK_Error,
    /// The identifier is a tentatively-declared name.
    ANK_TentativeDecl,
    /// The identifier can't be resolved.
    ANK_Unresolved,
    /// Annotation was successful.
    ANK_Success
  };
  // AnnotatedNameKind TryAnnotateName(CorrectionCandidateCallback *CCC =
  // nullptr);

  /// Push a tok::annot_cxxscope token onto the token stream.
  void AnnotateScopeToken(CXXScopeSpec &SS, bool IsNewAnnotation);

  /// TryKeywordIdentFallback - For compatibility with system headers using
  /// keywords as identifiers, attempt to convert the current token to an
  /// identifier and optionally disable the keyword for the remainder of the
  /// translation unit. This returns false if the token was not replaced,
  /// otherwise emits a diagnostic and returns true.
  bool TryKeywordIdentFallback(bool DisableKeyword);

  /// ExpectAndConsume - The parser expects that 'ExpectedTok' is next in the
  /// input.  If so, it is consumed and false is returned.
  ///
  /// If a trivial punctuator misspelling is encountered, a FixIt error
  /// diagnostic is issued and false is returned after recovery.
  ///
  /// If the input is malformed, this emits the specified diagnostic and true is
  /// returned.
  bool ExpectAndConsume(tok::TokenKind ExpectedTok,
                        /*unsigned Diag = diag::err_expected,*/
                        llvm::StringRef DiagMsg = "");

  /// The parser expects a semicolon and, if present, will consume it.
  ///
  /// If the next token is not a semicolon, this emits the specified diagnostic,
  /// or, if there's just some closing-delimiter noise (e.g., ')' or ']') prior
  /// to the semicolon, consumes that extra token.
  bool ExpectAndConsumeSemi(unsigned DiagID);

  /// The kind of extra semi diagnostic to emit.
  enum ExtraSemiKind {
    OutsideFunction = 0,
    InsideStruct = 1,
    InstanceVariableList = 2,
    AfterMemberFunctionDefinition = 3
  };

  /// Consume any extra semi-colons until the end of the line.
  void ConsumeExtraSemi(ExtraSemiKind Kind, DeclSpec::TST T = TST_unspecified);

  /// Return false if the next token is an identifier. An 'expected identifier'
  /// error is emitted otherwise.
  ///
  /// The parser tries to recover from the error by checking if the next token
  /// is a C++ keyword when parsing Objective-C++. Return false if the recovery
  /// was successful.
  bool expectIdentifier();

public:
  //===--------------------------------------------------------------------===//
  // Scope manipulation

  /// ParseScope - Introduces a new scope for parsing. The kind of
  /// scope is determined by ScopeFlags. Objects of this type should
  /// be created on the stack to coincide with the position where the
  /// parser enters the new scope, and this object's constructor will
  /// create that new scope. Similarly, once the object is destroyed
  /// the parser will exit the scope.
  class ParseScope {
    Parser *Self;
    ParseScope(const ParseScope &) = delete;
    void operator=(const ParseScope &) = delete;

  public:
    // ParseScope - Construct a new object to manage a scope in the
    // parser Self where the new Scope is created with the flags
    // ScopeFlags, but only when we aren't about to enter a compound statement.
    ParseScope(Parser *Self, unsigned ScopeFlags, bool EnteredScope = true,
               bool BeforeCompoundStmt = false)
        : Self(Self) {
      if (EnteredScope && !BeforeCompoundStmt)
        Self->EnterScope(ScopeFlags);
      else {
        // if (BeforeCompoundStmt)
        //   Self->incrementMSManglingNumber();

        this->Self = nullptr;
      }
    }

    // Exit - Exit the scope associated with this object now, rather
    // than waiting until the object is destroyed.
    void Exit() {
      if (Self) {
        Self->ExitScope();
        Self = nullptr;
      }
    }

    ~ParseScope() { Exit(); }
  };

  /// Introduces zero or more scopes for parsing. The scopes will all be exited
  /// when the object is destroyed.
  class MultiParseScope {
    Parser &Self;
    unsigned NumScopes = 0;

    MultiParseScope(const MultiParseScope &) = delete;

  public:
    MultiParseScope(Parser &Self) : Self(Self) {}
    void Enter(unsigned ScopeFlags) { ++NumScopes; }
    void Exit() {
      while (NumScopes) {
        Self.ExitScope();
        --NumScopes;
      }
    }
    ~MultiParseScope() { Exit(); }
  };

  /// EnterScope - Start a new scope.
  void EnterScope(unsigned ScopeFlags);

  /// ExitScope - Pop a scope off the scope stack.
  void ExitScope();

private:
  /// RAII object used to modify the scope flags for the current scope.
  class ParseScopeFlags {
    Scope *CurScope;
    unsigned OldFlags;
    ParseScopeFlags(const ParseScopeFlags &) = delete;
    void operator=(const ParseScopeFlags &) = delete;

  public:
    ParseScopeFlags(Parser *Self, unsigned ScopeFlags, bool ManageFlags = true);
    ~ParseScopeFlags();
  };

  //===--------------------------------------------------------------------===//
  // Diagnostic Emission and Error recovery.

public:
  DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID);
  DiagnosticBuilder Diag(const Token &Tok, unsigned DiagID);
  DiagnosticBuilder Diag(unsigned DiagID) { return Diag(Tok, DiagID); }

private:
  void SuggestParentheses(SourceLocation Loc, unsigned DK,
                          SourceRange ParenRange);
  // void CheckNestedObjCContexts(SourceLocation AtLoc);

public:
  /// Control flags for SkipUntil functions.
  enum SkipUntilFlags {
    StopAtSemi = 1 << 0, ///< Stop skipping at semicolon
    /// Stop skipping at specified token, but don't skip the token itself
    StopBeforeMatch = 1 << 1,
    StopAtCodeCompletion = 1 << 2 ///< Stop at code completion
  };

  friend constexpr SkipUntilFlags operator|(SkipUntilFlags L,
                                            SkipUntilFlags R) {
    return static_cast<SkipUntilFlags>(static_cast<unsigned>(L) |
                                       static_cast<unsigned>(R));
  }

  /// SkipUntil - Read tokens until we get to the specified token, then consume
  /// it (unless StopBeforeMatch is specified).  Because we cannot guarantee
  /// that the token will ever occur, this skips to the next token, or to some
  /// likely good stopping point.  If Flags has StopAtSemi flag, skipping will
  /// stop at a ';' character. Balances (), [], and {} delimiter tokens while
  /// skipping.
  ///
  /// If SkipUntil finds the specified token, it returns true, otherwise it
  /// returns false.
  bool SkipUntil(tok::TokenKind T,
                 SkipUntilFlags Flags = static_cast<SkipUntilFlags>(0)) {
    return SkipUntil(llvm::makeArrayRef(T), Flags);
  }
  bool SkipUntil(tok::TokenKind T1, tok::TokenKind T2,
                 SkipUntilFlags Flags = static_cast<SkipUntilFlags>(0)) {
    tok::TokenKind TokArray[] = {T1, T2};
    return SkipUntil(TokArray, Flags);
  }
  bool SkipUntil(tok::TokenKind T1, tok::TokenKind T2, tok::TokenKind T3,
                 SkipUntilFlags Flags = static_cast<SkipUntilFlags>(0)) {
    tok::TokenKind TokArray[] = {T1, T2, T3};
    return SkipUntil(TokArray, Flags);
  }
  bool SkipUntil(ArrayRef<tok::TokenKind> Toks,
                 SkipUntilFlags Flags = static_cast<SkipUntilFlags>(0));

  /// SkipMalformedDecl - Read tokens until we get to some likely good stopping
  /// point for skipping past a simple-declaration.
  void SkipMalformedDecl();

  /// The location of the first statement inside an else that might
  /// have a missleading indentation. If there is no
  /// MisleadingIndentationChecker on an else active, this location is invalid.
  SourceLocation MisleadingIndentationElseLoc;

private:
  //===--------------------------------------------------------------------===//
  // Lexing and parsing of C++ inline methods.

  struct ParsingClass;

  /// [class.mem]p1: "... the class is regarded as complete within
  /// - function bodies
  /// - default arguments
  /// - exception-specifications (TODO: C++0x)
  /// - and brace-or-equal-initializers for non-static data members
  /// (including such things in nested classes)."
  /// LateParsedDeclarations build the tree of those elements so they can
  /// be parsed after parsing the top-level class.
  class LateParsedDeclaration {
  public:
    virtual ~LateParsedDeclaration();

    virtual void ParseLexedMethodDeclarations();
    virtual void ParseLexedMemberInitializers();
    virtual void ParseLexedMethodDefs();
    // virtual void ParseLexedAttributes();
    // virtual void ParseLexedPragmas();
  };

  /// Inner node of the LateParsedDeclaration tree that parses
  /// all its members recursively.
  class LateParsedClass : public LateParsedDeclaration {
  public:
    LateParsedClass(Parser *P, ParsingClass *C);
    ~LateParsedClass() override;

    void ParseLexedMethodDeclarations() override;
    void ParseLexedMemberInitializers() override;
    void ParseLexedMethodDefs() override;
    // void ParseLexedAttributes() override;
    // void ParseLexedPragmas() override;

  private:
    Parser *Self;
    ParsingClass *Class;
  };

  /// Contains the lexed tokens of a member function definition
  /// which needs to be parsed at the end of the class declaration
  /// after parsing all other member declarations.
  struct LexedMethod : public LateParsedDeclaration {
    Parser *Self;
    Decl *D;
    CachedTokens Toks;

    explicit LexedMethod(Parser *P, Decl *MD) : Self(P), D(MD) {}

    void ParseLexedMethodDefs() override;
  };

  /// LateParsedDefaultArgument - Keeps track of a parameter that may
  /// have a default argument that cannot be parsed yet because it
  /// occurs within a member function declaration inside the class
  /// (C++ [class.mem]p2).
  struct LateParsedDefaultArgument {
    explicit LateParsedDefaultArgument(
        Decl *P, std::unique_ptr<CachedTokens> Toks = nullptr)
        : Param(P), Toks(std::move(Toks)) {}

    /// Param - The parameter declaration for this parameter.
    Decl *Param;

    /// Toks - The sequence of tokens that comprises the default
    /// argument expression, not including the '=' or the terminating
    /// ')' or ','. This will be NULL for parameters that have no
    /// default argument.
    std::unique_ptr<CachedTokens> Toks;
  };

  /// LateParsedMethodDeclaration - A method declaration inside a class that
  /// contains at least one entity whose parsing needs to be delayed
  /// until the class itself is completely-defined, such as a default
  /// argument (C++ [class.mem]p2).
  struct LateParsedMethodDeclaration : public LateParsedDeclaration {
    explicit LateParsedMethodDeclaration(Parser *P, Decl *M)
        : Self(P), Method(M), ExceptionSpecTokens(nullptr) {}

    void ParseLexedMethodDeclarations() override;

    Parser *Self;

    /// Method - The method declaration.
    Decl *Method;

    /// DefaultArgs - Contains the parameters of the function and
    /// their default arguments. At least one of the parameters will
    /// have a default argument, but all of the parameters of the
    /// method will be stored so that they can be reintroduced into
    /// scope at the appropriate times.
    SmallVector<LateParsedDefaultArgument, 8> DefaultArgs;

    /// The set of tokens that make up an exception-specification that
    /// has not yet been parsed.
    CachedTokens *ExceptionSpecTokens;
  };

  /// LateParsedDeclarationsContainer - During parsing of a top (non-nested)
  /// C++ class, its method declarations that contain parts that won't be
  /// parsed until after the definition is completed (C++ [class.mem]p2),
  /// the method declarations and possibly attached inline definitions
  /// will be stored here with the tokens that will be parsed to create those
  /// entities.
  typedef SmallVector<LateParsedDeclaration *, 2>
      LateParsedDeclarationsContainer;

  /// LateParsedMemberInitializer - An initializer for a non-static class data
  /// member whose parsing must to be delayed until the class is completely
  /// defined (C++11 [class.mem]p2).
  struct LateParsedMemberInitializer : public LateParsedDeclaration {
    LateParsedMemberInitializer(Parser *P, Decl *FD) : Self(P), Field(FD) {}

    void ParseLexedMemberInitializers() override;

    Parser *Self;

    /// Field - The field declaration.
    Decl *Field;

    /// CachedTokens - The sequence of tokens that comprises the initializer,
    /// including any leading '='.
    CachedTokens Toks;
  };

  /// LateParsedDeclarationsContainer - During parsing of a top (non-nested)
  /// C++ class, its method declarations that contain parts that won't be
  /// parsed until after the definition is completed (C++ [class.mem]p2),
  /// the method declarations and possibly attached inline definitions
  /// will be stored here with the tokens that will be parsed to create those
  /// entities.
  typedef SmallVector<LateParsedDeclaration *, 2>
      LateParsedDeclarationsContainer;

  /// The stack of classes that is currently being
  /// parsed. Nested and local classes will be pushed onto this stack
  /// when they are parsed, and removed afterward.
  std::stack<ParsingClass *> ClassStack;

  ParsingClass &getCurrentClass() {
    assert(!ClassStack.empty() && "No lexed method stacks!");
    return *ClassStack.top();
  }

  enum CachedInitKind { CIK_DefaultArgument, CIK_DefaultInitializer };

  void ParseCXXNonStaticMemberInitializer(Decl *VarD);
  void ParseLexedMethodDeclarations(ParsingClass &Class);
  void ParseLexedMethodDeclaration(LateParsedMethodDeclaration &LM);
  void ParseLexedMethodDefs(ParsingClass &Class);
  void ParseLexedMethodDef(LexedMethod &LM);
  void ParseLexedMemberInitializers(ParsingClass &Class);
  void ParseLexedMemberInitializer(LateParsedMemberInitializer &MI);
  // void ParseLexedPragmas(ParsingClass &Class);
  bool ConsumeAndStoreFunctionPrologue(CachedTokens &Toks);
  bool ConsumeAndStoreInitializer(CachedTokens &Toks, CachedInitKind CIK);
  bool ConsumeAndStoreConditional(CachedTokens &Toks);
  bool ConsumeAndStoreUntil(tok::TokenKind T1, CachedTokens &Toks,
                            bool StopAtSemi = true,
                            bool ConsumeFinalToken = true) {
    return ConsumeAndStoreUntil(T1, T1, Toks, StopAtSemi, ConsumeFinalToken);
  }
  bool ConsumeAndStoreUntil(tok::TokenKind T1, tok::TokenKind T2,
                            CachedTokens &Toks, bool StopAtSemi = true,
                            bool ConsumeFinalToken = true);

  bool isDeclarationAfterDeclarator();
  bool isStartOfFunctionDefinition(const ParsingDeclarator &Declarator);
  DeclGroupPtrTy
  ParseDeclarationOrFunctionDefinition(/*ParsedAttributesWithRange &attrs,*/
                                       ParsingDeclSpec *DS = nullptr,
                                       AccessSpecifier AS = AS_none);
  DeclGroupPtrTy
  ParseDeclOrFunctionDefInternal(/*ParsedAttributesWithRange &attrs,*/
                                 ParsingDeclSpec &DS, AccessSpecifier AS);

  void SkipFunctionBody();
  Decl *ParseFunctionDefinition(ParsingDeclarator &D);
  void ParseKNRParamDeclarations(Declarator &D);

public:
  //===--------------------------------------------------------------------===//
  // C99 6.5: Expressions.

  /// TypeCastState - State whether an expression is or may be a type cast.
  enum TypeCastState { NotTypeCast = 0, MaybeTypeCast, IsTypeCast };

  ExprResult ParseExpression(TypeCastState isTypeCast = NotTypeCast);
  ExprResult ParseConstantExpressionInExprEvalContext(
      TypeCastState isTypeCast = NotTypeCast);
  ExprResult ParseConstantExpression(TypeCastState isTypeCast = NotTypeCast);
  ExprResult ParseCaseExpression(SourceLocation CaseLoc);
  ExprResult ParseConstraintExpression();
  ExprResult ParseConstraintLogicalAndExpression(bool IsTrailingRequiresClause);
  ExprResult ParseConstraintLogicalOrExpression(bool IsTrailingRequiresClause);
  ExprResult ParseStringLiteralExpression(bool AllowUserDefinedLiteral = false);

  // Expr that doesn't include commas.
  ExprResult ParseAssignmentExpression(TypeCastState isTypeCast = NotTypeCast);

private:
  ExprResult ParseExpressionWithLeadingAt(SourceLocation AtLoc);

  ExprResult ParseExpressionWithLeadingExtension(SourceLocation ExtLoc);
  ExprResult ParseRHSOfBinaryExpression(ExprResult LHS, prec::Level MinPrec);

  /// Control what ParseCastExpression will parse.
  enum CastParseKind { AnyCastExpr = 0, UnaryExprOnly, PrimaryExprOnly };
  ExprResult ParseCastExpression(CastParseKind ParseKind,
                                 bool isAddressOfOperand, bool &NotCastExpr,
                                 TypeCastState isTypeCast,
                                 bool isVectorLiteral = false,
                                 bool *NotPrimaryExpression = nullptr);
  ExprResult ParseCastExpression(CastParseKind ParseKind,
                                 bool isAddressOfOperand = false,
                                 TypeCastState isTypeCast = NotTypeCast,
                                 bool isVectorLiteral = false,
                                 bool *NotPrimaryExpression = nullptr);

  /// Returns true if the next token cannot start an expression.
  bool isNotExpressionStart();

  /// Returns true if the next token would start a postfix-expression
  /// suffix.
  bool isPostfixExpressionSuffixStart() {
    tok::TokenKind K = Tok.getKind();
    return (K == tok::l_square || K == tok::l_paren || K == tok::period ||
            /*K == tok::arrow ||*/ K == tok::plusplus || K == tok::minusminus);
  }

  bool checkPotentialAngleBracketDelimiter(const AngleBracketTracker::Loc &,
                                           const Token &OpToken);
  bool checkPotentialAngleBracketDelimiter(const Token &OpToken) {
    if (auto *Info = AngleBrackets.getCurrent(*this))
      return checkPotentialAngleBracketDelimiter(*Info, OpToken);
    return false;
  }

  ExprResult ParsePostfixExpressionSuffix(ExprResult LHS);
  ExprResult ParseUnaryExprOrTypeTraitExpression();
  ExprResult ParseBuiltinPrimaryExpression();
  ExprResult ParseUniqueStableNameExpression();

  ExprResult ParseExprAfterUnaryExprOrTypeTrait(const Token &OpTok,
                                                bool &isCastExpr,
                                                ParsedType &CastTy,
                                                SourceRange &CastRange);

  typedef SmallVector<Expr *, 20> ExprListTy;
  typedef SmallVector<SourceLocation, 20> CommaLocsTy;

  /// ParseExpressionList - Used for C/C++ (argument-)expression-list.
  bool ParseExpressionList(SmallVectorImpl<Expr *> &Exprs,
                           SmallVectorImpl<SourceLocation> &CommaLocs,
                           llvm::function_ref<void()> ExpressionStarts =
                               llvm::function_ref<void()>());

  /// ParseSimpleExpressionList - A simple comma-separated list of expressions,
  /// used for misc language extensions.
  bool ParseSimpleExpressionList(SmallVectorImpl<Expr *> &Exprs,
                                 SmallVectorImpl<SourceLocation> &CommaLocs);

  /// ParenParseOption - Control what ParseParenExpression will parse.
  enum ParenParseOption {
    SimpleExpr,      // Only parse '(' expression ')'
    FoldExpr,        // Also allow fold-expression <anything>
    CompoundStmt,    // Also allow '(' compound-statement ')'
    CompoundLiteral, // Also allow '(' type-name ')' '{' ... '}'
    CastExpr         // Also allow '(' type-name ')' <anything>
  };
  ExprResult ParseParenExpression(ParenParseOption &ExprType,
                                  bool stopIfCastExpr, bool isTypeCast,
                                  ParsedType &CastTy,
                                  SourceLocation &RParenLoc);

  ExprResult ParseCXXAmbiguousParenExpression(
      ParenParseOption &ExprType, ParsedType &CastTy,
      BalancedDelimiterTracker &Tracker, ColonProtectionRAIIObject &ColonProt);
  ExprResult ParseCompoundLiteralExpression(ParsedType Ty,
                                            SourceLocation LParenLoc,
                                            SourceLocation RParenLoc);

  ExprResult ParseFoldExpression(ExprResult LHS, BalancedDelimiterTracker &T);

  //===--------------------------------------------------------------------===//
  // C++ Expressions
  ExprResult tryParseCXXIdExpression(CXXScopeSpec &SS, bool isAddressOfOperand,
                                     Token &Replacement);
  ExprResult ParseCXXIdExpression(bool isAddressOfOperand = false);

  bool areTokensAdjacent(const Token &A, const Token &B);

  bool ParseOptionalCXXScopeSpecifier(
      CXXScopeSpec &SS, ParsedType ObjectType, bool ObjectHasErrors,
      bool EnteringContext, bool *MayBePseudoDestructor = nullptr,
      bool IsTypename = false, IdentifierInfo **LastII = nullptr,
      bool OnlyNamespace = false, bool InUsingDeclaration = false);

  //===--------------------------------------------------------------------===//
  // C++ 5.2p1: C++ Casts
  ExprResult ParseCXXCasts();

  /// Parse a __builtin_bit_cast(T, E), used to implement C++2a std::bit_cast.
  ExprResult ParseBuiltinBitCast();

  //===--------------------------------------------------------------------===//
  // C++ 5.2p1: C++ Type Identification
  ExprResult ParseCXXTypeid();

  //===--------------------------------------------------------------------===//
  //  C++ : Microsoft __uuidof Expression
  ExprResult ParseCXXUuidof();

  //===--------------------------------------------------------------------===//
  // C++ 5.2.4: C++ Pseudo-Destructor Expressions
  ExprResult ParseCXXPseudoDestructor(Expr *Base, SourceLocation OpLoc,
                                      tok::TokenKind OpKind, CXXScopeSpec &SS,
                                      ParsedType ObjectType);

  //===--------------------------------------------------------------------===//
  // C++ 9.3.2: C++ 'this' pointer
  ExprResult ParseCXXThis();

  //===--------------------------------------------------------------------===//
  // C++ 15: C++ Throw Expression
  ExprResult ParseThrowExpression();

  ExceptionSpecificationType tryParseExceptionSpecification(
      bool Delayed, SourceRange &SpecificationRange,
      SmallVectorImpl<ParsedType> &DynamicExceptions,
      SmallVectorImpl<SourceRange> &DynamicExceptionRanges,
      ExprResult &NoexceptExpr, CachedTokens *&ExceptionSpecTokens);

  // EndLoc is filled with the location of the last token of the specification.
  ExceptionSpecificationType
  ParseDynamicExceptionSpecification(SourceRange &SpecificationRange,
                                     SmallVectorImpl<ParsedType> &Exceptions,
                                     SmallVectorImpl<SourceRange> &Ranges);

  //===--------------------------------------------------------------------===//
  // C++0x 8: Function declaration trailing-return-type
  TypeResult ParseTrailingReturnType(SourceRange &Range,
                                     bool MayBeFollowedByDirectInit);

  //===--------------------------------------------------------------------===//
  // C++ 2.13.5: C++ Boolean Literals
  ExprResult ParseCXXBoolLiteral();

  //===--------------------------------------------------------------------===//
  // C++ 5.2.3: Explicit type conversion (functional notation)
  ExprResult ParseCXXTypeConstructExpression(const DeclSpec &DS);

  /// ParseCXXSimpleTypeSpecifier - [C++ 7.1.5.2] Simple type specifiers.
  /// This should only be called when the current token is known to be part of
  /// simple-type-specifier.
  void ParseCXXSimpleTypeSpecifier(DeclSpec &DS);

  bool ParseCXXTypeSpecifierSeq(DeclSpec &DS);

  //===--------------------------------------------------------------------===//
  // C++ 5.3.4 and 5.3.5: C++ new and delete
  bool ParseExpressionListOrTypeId(SmallVectorImpl<Expr *> &Exprs,
                                   Declarator &D);
  void ParseDirectNewDeclarator(Declarator &D);
  ExprResult ParseCXXNewExpression(bool UseGlobal, SourceLocation Start);
  ExprResult ParseCXXDeleteExpression(bool UseGlobal, SourceLocation Start);

  //===--------------------------------------------------------------------===//
  // C++ if/switch/while/for condition expression.
  struct ForRangeInfo;
  Sema::ConditionResult ParseCXXCondition(StmtResult *InitStmt,
                                          SourceLocation Loc,
                                          Sema::ConditionKind CK,
                                          ForRangeInfo *FRI = nullptr);

  //===--------------------------------------------------------------------===//
  // C99 6.7.8: Initialization.

  /// ParseInitializer
  ///       initializer: [C99 6.7.8]
  ///         assignment-expression
  ///         '{' ...
  ExprResult ParseInitializer() {
    if (Tok.isNot(tok::l_brace))
      return ParseAssignmentExpression();
    return ParseBraceInitializer();
  }
  bool MayBeDesignationStart();
  ExprResult ParseBraceInitializer();
  ExprResult ParseInitializerWithPotentialDesignator(
      llvm::function_ref<void(const Designation &)> CodeCompleteCB);

  //===--------------------------------------------------------------------===//
  // clang Expressions

  ExprResult ParseBlockLiteralExpression(); // ^{...}

  //===--------------------------------------------------------------------===//
  // C99 6.8: Statements and Blocks.

  /// A SmallVector of statements, with stack size 32 (as that is the only one
  /// used.)
  typedef llvm::SmallVector<Stmt *, 32> StmtVector;
  /// A SmallVector of expressions, with stack size 12 (the maximum used.)
  typedef llvm::SmallVector<Expr *, 12> ExprVector;
  /// A SmallVector of types.
  // typedef llvm::SmallVector<ParsedType, 12> TypeVector;

  StmtResult
  ParseStatement(SourceLocation *TrailingElseLoc = nullptr,
                 ParsedStmtContext StmtCtx = ParsedStmtContext::SubStmt);

  StmtResult
  ParseStatementOrDeclaration(StmtVector &Stmts, ParsedStmtContext StmtCtx,
                              SourceLocation *TrailingElseLoc = nullptr);

  StmtResult ParseStatementOrDeclarationAfterAttributes(
      StmtVector &Stmts, ParsedStmtContext StmtCtx,
      SourceLocation *TrailingElseLoc /*, ParsedAttributesWithRange &Attrs*/);

  StmtResult ParseExprStatement(ParsedStmtContext StmtCtx);

  StmtResult ParseLabeledStatement(/*ParsedAttributesWithRange &attrs,*/
                                   ParsedStmtContext StmtCtx);

  StmtResult ParseCaseStatement(ParsedStmtContext StmtCtx,
                                bool MissingCase = false,
                                ExprResult Expr = ExprResult());

  StmtResult ParseDefaultStatement(ParsedStmtContext StmtCtx);
  StmtResult ParseCompoundStatement(bool isStmtExpr = false);
  StmtResult ParseCompoundStatement(bool isStmtExpr, unsigned ScopeFlags);
  bool ConsumeNullStmt(StmtVector &Stmts);
  StmtResult ParseCompoundStatementBody(bool isStmtExpr = false);
  bool ParseParenExprOrCondition(StmtResult *InitStmt,
                                 Sema::ConditionResult &CondResult,
                                 SourceLocation Loc, Sema::ConditionKind CK,
                                 SourceLocation *LParenLoc = nullptr,
                                 SourceLocation *RParenLoc = nullptr);
  StmtResult ParseIfStatement(SourceLocation *TrailingElseLoc);
  StmtResult ParseSwitchStatement(SourceLocation *TrailingElseLoc);
  StmtResult ParseWhileStatement(SourceLocation *TrailingElseLoc);
  StmtResult ParseDoStatement();
  StmtResult ParseForStatement(SourceLocation *TrailingElseLoc);
  StmtResult ParseGotoStatement();
  StmtResult ParseContinueStatement();
  StmtResult ParseBreakStatement();
  StmtResult ParseReturnStatement();

  //===--------------------------------------------------------------------===//
  // C++ 6: Statements and Blocks

  StmtResult ParseCXXTryBlock();
  StmtResult ParseCXXTryBlockCommon(SourceLocation TryLoc, bool FnTry = false);
  StmtResult ParseCXXCatchBlock(bool FnCatch = false);

  //===--------------------------------------------------------------------===//
  // C99 6.7: Declarations.

  /// A context for parsing declaration specifiers.  TODO: flesh this
  /// out, there are other significant restrictions on specifiers than
  /// would be best implemented in the parser.
  enum class DeclSpecContext {
    DSC_normal,         // normal context
    DSC_class,          // class context, enables 'friend'
    DSC_type_specifier, // C++ type-specifier-seq or C specifier-qualifier-list
    DSC_trailing, // C++11 trailing-type-specifier in a trailing return type
    DSC_alias_declaration, // C++11 type-specifier-seq in an alias-declaration
    DSC_top_level,         // top-level/namespace declaration context
    DSC_template_param,    // template parameter context
    DSC_template_type_arg, // template type argument context
    // DSC_objc_method_result, // ObjC method result context, enables
    // 'instancetype'
    DSC_condition // condition declaration context
  };

  /// Is this a context in which we are parsing just a type-specifier (or
  /// trailing-type-specifier)?
  static bool isTypeSpecifier(DeclSpecContext DSC) {
    switch (DSC) {
    case DeclSpecContext::DSC_normal:
    case DeclSpecContext::DSC_template_param:
    case DeclSpecContext::DSC_class:
    case DeclSpecContext::DSC_top_level:
    // case DeclSpecContext::DSC_objc_method_result:
    case DeclSpecContext::DSC_condition:
      return false;

    case DeclSpecContext::DSC_template_type_arg:
    case DeclSpecContext::DSC_type_specifier:
    case DeclSpecContext::DSC_trailing:
    case DeclSpecContext::DSC_alias_declaration:
      return true;
    }
    llvm_unreachable("Missing DeclSpecContext case");
  }

  /// Whether a defining-type-specifier is permitted in a given context.
  enum class AllowDefiningTypeSpec {
    /// The grammar doesn't allow a defining-type-specifier here, and we must
    /// not parse one (eg, because a '{' could mean something else).
    No,
    /// The grammar doesn't allow a defining-type-specifier here, but we permit
    /// one for error recovery purposes. Sema will reject.
    NoButErrorRecovery,
    /// The grammar allows a defining-type-specifier here, even though it's
    /// always invalid. Sema will reject.
    YesButInvalid,
    /// The grammar allows a defining-type-specifier here, and one can be valid.
    Yes
  };

  /// Is this a context in which we are parsing defining-type-specifiers (and
  /// so permit class and enum definitions in addition to non-defining class and
  /// enum elaborated-type-specifiers)?
  static AllowDefiningTypeSpec
  isDefiningTypeSpecifierContext(DeclSpecContext DSC) {
    switch (DSC) {
    case DeclSpecContext::DSC_normal:
    case DeclSpecContext::DSC_class:
    case DeclSpecContext::DSC_top_level:
    case DeclSpecContext::DSC_alias_declaration:
      // case DeclSpecContext::DSC_objc_method_result:
      return AllowDefiningTypeSpec::Yes;

    case DeclSpecContext::DSC_condition:
    case DeclSpecContext::DSC_template_param:
      return AllowDefiningTypeSpec::YesButInvalid;

    case DeclSpecContext::DSC_template_type_arg:
    case DeclSpecContext::DSC_type_specifier:
      return AllowDefiningTypeSpec::NoButErrorRecovery;

    case DeclSpecContext::DSC_trailing:
      return AllowDefiningTypeSpec::No;
    }
    llvm_unreachable("Missing DeclSpecContext case");
  }

  /// Is this a context in which an opaque-enum-declaration can appear?
  static bool isOpaqueEnumDeclarationContext(DeclSpecContext DSC) {
    switch (DSC) {
    case DeclSpecContext::DSC_normal:
    case DeclSpecContext::DSC_class:
    case DeclSpecContext::DSC_top_level:
      return true;

    case DeclSpecContext::DSC_alias_declaration:
    // case DeclSpecContext::DSC_objc_method_result:
    case DeclSpecContext::DSC_condition:
    case DeclSpecContext::DSC_template_param:
    case DeclSpecContext::DSC_template_type_arg:
    case DeclSpecContext::DSC_type_specifier:
    case DeclSpecContext::DSC_trailing:
      return false;
    }
    llvm_unreachable("Missing DeclSpecContext case");
  }

  /// Information on a C++0x for-range-initializer found while parsing a
  /// declaration which turns out to be a for-range-declaration.
  struct ForRangeInit {
    SourceLocation ColonLoc;
    ExprResult RangeExpr;

    bool ParsedForRangeDecl() { return !ColonLoc.isInvalid(); }
  };
  struct ForRangeInfo : ForRangeInit {
    StmtResult LoopVar;
  };

  DeclGroupPtrTy ParseDeclaration(DeclaratorContext Context,
                                  SourceLocation &DeclEnd,
                                  // ParsedAttributesWithRange &attrs,
                                  SourceLocation *DeclSpecStart = nullptr);
  DeclGroupPtrTy
  ParseSimpleDeclaration(DeclaratorContext Context, SourceLocation &DeclEnd,
                         //  ParsedAttributesWithRange &attrs,
                         bool RequireSemi, ForRangeInit *FRI = nullptr,
                         SourceLocation *DeclSpecStart = nullptr);
  bool MightBeDeclarator(DeclaratorContext Context);
  DeclGroupPtrTy ParseDeclGroup(ParsingDeclSpec &DS, DeclaratorContext Context,
                                SourceLocation *DeclEnd = nullptr,
                                ForRangeInit *FRI = nullptr);

  Decl *ParseFunctionStatementBody(Decl *Decl, ParseScope &BodyScope);
  Decl *ParseFunctionTryBlock(Decl *Decl, ParseScope &BodyScope);

  /// When in code-completion, skip parsing of the function/method body
  /// unless the body contains the code-completion point.
  ///
  /// \returns true if the function body was skipped.
  bool trySkippingFunctionBody();

  DeclSpecContext
  getDeclSpecContextFromDeclaratorContext(DeclaratorContext Context);
  void
  ParseDeclarationSpecifiers(DeclSpec &DS, AccessSpecifier AS = AS_none,
                             DeclSpecContext DSC = DeclSpecContext::DSC_normal/*,
                             LateParsedAttrList *LateAttrs = nullptr*/);
  bool DiagnoseMissingSemiAfterTagDefinition(
      DeclSpec &DS, AccessSpecifier AS, DeclSpecContext DSContext/*,
      LateParsedAttrList *LateAttrs = nullptr*/);

  void ParseSpecifierQualifierList(
      DeclSpec &DS, AccessSpecifier AS = AS_none,
      DeclSpecContext DSC = DeclSpecContext::DSC_normal);

  void ParseEnumBody(SourceLocation StartLoc, Decl *TagDecl);
  void ParseStructUnionBody(SourceLocation StartLoc, DeclSpec::TST TagType,
                            RecordDecl *TagDecl);

  void ParseStructDeclaration(
      ParsingDeclSpec &DS,
      llvm::function_ref<void(ParsingFieldDeclarator &)> FieldsCallback);

  bool isDeclarationSpecifier(bool DisambiguatingWithExpression = false);
  bool isTypeSpecifierQualifier();

  /// Return true if we know that we are definitely looking at a
  /// decl-specifier, and isn't part of an expression such as a function-style
  /// cast. Return false if it's no a decl-specifier, or we're not sure.
  bool isKnownToBeDeclarationSpecifier() {
    // if (getLangOpts().CPlusPlus)
    //   return isCXXDeclarationSpecifier() == TPResult::True;
    return isDeclarationSpecifier(true);
  }

  /// isDeclarationStatement - Disambiguates between a declaration or an
  /// expression statement, when parsing function bodies.
  /// Returns true for declaration, false for expression.
  bool isDeclarationStatement() {
    if (getLangOpts().CPlusPlus)
      return isCXXDeclarationStatement();
    return isDeclarationSpecifier(true);
  }

  /// isForInitDeclaration - Disambiguates between a declaration or an
  /// expression in the context of the C 'clause-1' or the C++
  // 'for-init-statement' part of a 'for' statement.
  /// Returns true for declaration, false for expression.
  bool isForInitDeclaration() {
    /*if (getLangOpts().OpenMP)
      Actions.startOpenMPLoop();*/
    if (getLangOpts().CPlusPlus)
      return isCXXSimpleDeclaration(/*AllowForRangeDecl=*/true);
    return isDeclarationSpecifier(true);
  }

  /// Determine whether this is a C++1z for-range-identifier.
  bool isForRangeIdentifier();

  /// Starting with a scope specifier, identifier, or
  /// template-id that refers to the current class, determine whether
  /// this is a constructor declarator.
  bool isConstructorDeclarator(bool Unqualified, bool DeductionGuide = false);

  /// Specifies the context in which type-id/expression
  /// disambiguation will occur.
  enum TentativeCXXTypeIdContext { TypeIdInParens, TypeIdUnambiguous };

  /// isTypeIdInParens - Assumes that a '(' was parsed and now we want to know
  /// whether the parens contain an expression or a type-id.
  /// Returns true for a type-id and false for an expression.
  bool isTypeIdInParens(bool &isAmbiguous) {
    if (getLangOpts().CPlusPlus)
      return isCXXTypeId(TypeIdInParens, isAmbiguous);
    isAmbiguous = false;
    return isTypeSpecifierQualifier();
  }
  bool isTypeIdInParens() {
    bool isAmbiguous;
    return isTypeIdInParens(isAmbiguous);
  }

  /// Checks if the current tokens form type-id or expression.
  /// It is similar to isTypeIdInParens but does not suppose that type-id
  /// is in parenthesis.
  bool isTypeIdUnambiguously() {
    bool IsAmbiguous;
    if (getLangOpts().CPlusPlus)
      return isCXXTypeId(TypeIdUnambiguous, IsAmbiguous);
    return isTypeSpecifierQualifier();
  }

  /// isCXXDeclarationStatement - C++-specialized function that disambiguates
  /// between a declaration or an expression statement, when parsing function
  /// bodies. Returns true for declaration, false for expression.
  bool isCXXDeclarationStatement();

  /// isCXXSimpleDeclaration - C++-specialized function that disambiguates
  /// between a simple-declaration or an expression-statement.
  /// If during the disambiguation process a parsing error is encountered,
  /// the function returns true to let the declaration parsing code handle it.
  /// Returns false if the statement is disambiguated as expression.
  bool isCXXSimpleDeclaration(bool AllowForRangeDecl);

  /// isCXXFunctionDeclarator - Disambiguates between a function declarator or
  /// a constructor-style initializer, when parsing declaration statements.
  /// Returns true for function declarator and false for constructor-style
  /// initializer. Sets 'IsAmbiguous' to true to indicate that this declaration
  /// might be a constructor-style initializer.
  /// If during the disambiguation process a parsing error is encountered,
  /// the function returns true to let the declaration parsing code handle it.
  bool isCXXFunctionDeclarator(bool *IsAmbiguous = nullptr);

  struct ConditionDeclarationOrInitStatementState;
  enum class ConditionOrInitStatement {
    Expression,    ///< Disambiguated as an expression (either kind).
    ConditionDecl, ///< Disambiguated as the declaration form of condition.
    InitStmtDecl,  ///< Disambiguated as a simple-declaration init-statement.
    ForRangeDecl,  ///< Disambiguated as a for-range declaration.
    Error          ///< Can't be any of the above!
  };
  /// Disambiguates between the different kinds of things that can happen
  /// after 'if (' or 'switch ('. This could be one of two different kinds of
  /// declaration (depending on whether there is a ';' later) or an expression.
  ConditionOrInitStatement
  isCXXConditionDeclarationOrInitStatement(bool CanBeInitStmt,
                                           bool CanBeForRangeDecl);

  bool isCXXTypeId(TentativeCXXTypeIdContext Context, bool &isAmbiguous);
  bool isCXXTypeId(TentativeCXXTypeIdContext Context) {
    bool isAmbiguous;
    return isCXXTypeId(Context, isAmbiguous);
  }

  /// TPResult - Used as the result value for functions whose purpose is to
  /// disambiguate C++ constructs by "tentatively parsing" them.
  enum class TPResult { True, False, Ambiguous, Error };

  /// Determine whether we could have an enum-base.
  ///
  /// \p AllowSemi If \c true, then allow a ';' after the enum-base; otherwise
  /// only consider this to be an enum-base if the next token is a '{'.
  ///
  /// \return \c false if this cannot possibly be an enum base; \c true
  /// otherwise.
  bool isEnumBase(bool AllowSemi);

  /// isCXXDeclarationSpecifier - Returns TPResult::True if it is a
  /// declaration specifier, TPResult::False if it is not,
  /// TPResult::Ambiguous if it could be either a decl-specifier or a
  /// function-style cast, and TPResult::Error if a parsing error was
  /// encountered. If it could be a braced C++11 function-style cast, returns
  /// BracedCastResult.
  /// Doesn't consume tokens.
  TPResult
  isCXXDeclarationSpecifier(TPResult BracedCastResult = TPResult::False,
                            bool *InvalidAsDeclSpec = nullptr);

  /// Given that isCXXDeclarationSpecifier returns \c TPResult::True or
  /// \c TPResult::Ambiguous, determine whether the decl-specifier would be
  /// a type-specifier other than a cv-qualifier.
  bool isCXXDeclarationSpecifierAType();

  /// Determine whether an '(' after an 'explicit' keyword is part of a C++20
  /// 'explicit(bool)' declaration, in earlier language modes where that is an
  /// extension.
  TPResult isExplicitBool();

  /// Determine whether an identifier has been tentatively declared as a
  /// non-type. Such tentative declarations should not be found to name a type
  /// during a tentative parse, but also should not be annotated as a non-type.
  bool isTentativelyDeclared(IdentifierInfo *II);

public:
  TypeResult
  ParseTypeName(SourceRange *Range = nullptr,
                DeclaratorContext Context = DeclaratorContext::TypeNameContext,
                AccessSpecifier AS = AS_none, Decl **OwnedType = nullptr /*,
                ParsedAttributes *Attrs = nullptr*/);

private:
  void ParseBlockId(SourceLocation CaretLoc);

  void DiagnoseUnexpectedNamespace(NamedDecl *Context);

  DeclGroupPtrTy ParseNamespace(DeclaratorContext Context,
                                SourceLocation &DeclEnd,
                                SourceLocation InlineLoc = SourceLocation());

  struct InnerNamespaceInfo {
    SourceLocation NamespaceLoc;
    SourceLocation InlineLoc;
    SourceLocation IdentLoc;
    IdentifierInfo *Ident;
  };
  using InnerNamespaceInfoList = llvm::SmallVector<InnerNamespaceInfo, 4>;

  void ParseInnerNamespace(const InnerNamespaceInfoList &InnerNSs,
                           unsigned int index, SourceLocation &InlineLoc,
                           /* ParsedAttributes &attrs,*/
                           BalancedDelimiterTracker &Tracker);
  Decl *ParseLinkage(ParsingDeclSpec &DS, DeclaratorContext Context);
  Decl *ParseExportDeclaration();

  //===--------------------------------------------------------------------===//
  // C++ 9: classes [class] and C structs/unions.
  bool isValidAfterTypeSpecifier(bool CouldBeBitfield);
  void SkipCXXMemberSpecification(SourceLocation StartLoc,
                                  SourceLocation AttrFixitLoc, unsigned TagType,
                                  Decl *TagDecl);
  void ParseCXXMemberSpecification(SourceLocation StartLoc,
                                   SourceLocation AttrFixitLoc,
                                   /*ParsedAttributesWithRange &Attrs,*/
                                   unsigned TagType, Decl *TagDecl);
  ExprResult ParseCXXMemberInitializer(Decl *D, bool IsFunction,
                                       SourceLocation &EqualLoc);
  bool ParseCXXMemberDeclaratorBeforeInitializer(Declarator &DeclaratorInfo,
                                                 VirtSpecifiers &VS,
                                                 ExprResult &BitfieldSize/*,
                                                 LateParsedAttrList &LateAttrs*/);
  void
  MaybeParseAndDiagnoseDeclSpecAfterCXX11VirtSpecifierSeq(Declarator &D,
                                                          VirtSpecifiers &VS);

  void ParseConstructorInitializer(Decl *ConstructorDecl);
  // MemInitResult ParseMemInitializer(Decl *ConstructorDecl);
  void HandleMemberFunctionDeclDelays(Declarator &DeclaratorInfo,
                                      Decl *ThisDecl);

  //===--------------------------------------------------------------------===//
  // C++ 10: Derived classes [class.derived]
  TypeResult ParseBaseTypeSpecifier(SourceLocation &BaseLoc,
                                    SourceLocation &EndLocation);
  void ParseBaseClause(Decl *ClassDecl);
  // BaseResult ParseBaseSpecifier(Decl *ClassDecl);
  AccessSpecifier getAccessSpecifierIfPresent() const;

  bool ParseUnqualifiedIdOperator(CXXScopeSpec &SS, bool EnteringContext,
                                  ParsedType ObjectType, UnqualifiedId &Result);

private:
  //===--------------------------------------------------------------------===//
  // TPResult isStartOfTemplateTypeParameter();
  // NamedDecl *ParseTemplateParameter(unsigned Depth, unsigned Position);
  // NamedDecl *ParseTypeParameter(unsigned Depth, unsigned Position);
  // NamedDecl *ParseTemplateTemplateParameter(unsigned Depth, unsigned
  // Position); NamedDecl *ParseNonTypeTemplateParameter(unsigned Depth,
  // unsigned Position);
  bool isTypeConstraintAnnotation();
  bool TryAnnotateTypeConstraint();
  void DiagnoseMisplacedEllipsis(SourceLocation EllipsisLoc,
                                 SourceLocation CorrectLoc,
                                 bool AlreadyHasEllipsis,
                                 bool IdentifierHasName);
  void DiagnoseMisplacedEllipsisInDeclarator(SourceLocation EllipsisLoc,
                                             Declarator &D);

  //===--------------------------------------------------------------------===//
  // Modules
  DeclGroupPtrTy ParseModuleDecl(bool IsFirstDecl);
  Decl *ParseModuleImport(SourceLocation AtLoc);
  bool parseMisplacedModuleImport();
  bool tryParseMisplacedModuleImport() {
    tok::TokenKind Kind = Tok.getKind();
    if (Kind == tok::annot_module_begin || Kind == tok::annot_module_end ||
        Kind == tok::annot_module_include)
      return parseMisplacedModuleImport();
    return false;
  }

  bool ParseModuleName(
      SourceLocation UseLoc,
      SmallVectorImpl<std::pair<IdentifierInfo *, SourceLocation>> &Path,
      bool IsImport);
};

} // namespace latino

#endif