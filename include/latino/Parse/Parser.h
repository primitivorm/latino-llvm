#ifndef LLVM_LATINO_PARSE_PARSER_H
#define LLVM_LATINO_PARSE_PARSER_H

#include "clang/Basic/BitmaskEnum.h"
#include "clang/Basic/Specifiers.h"
#include "clang/Sema/DeclSpec.h"
#include "clang/Sema/Sema.h"

#include "latino/Basic/OperatorPrecedence.h"
#include "latino/Lex/Preprocessor.h"
#include "latino/Sema/Sema.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Frontend/OpenMP/OMPContext.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/SaveAndRestore.h"
#include <memory>
#include <stack>

namespace latino {
class Parser {
  friend class BalancedDelimiterTracker;

  Preprocessor &PP;

  /// Tok - The current token we are peeking ahead.  All parsing methods assume
  /// that this is valid.
  Token Tok;

  // PrevTokLocation - The location of the token we previously
  // consumed. This token is used for diagnostics where we expected to
  // see a token following another token (e.g., the ';' at the end of
  // a statement).
  clang::SourceLocation PrevTokLocation;

  /// Tracks an expected type for the current token when parsing an expression.
  /// Used by code completion for ranking.
  // clang::PreferredTypeBuilder PreferredType;

  unsigned short ParenCount = 0, BracketCount = 0, BraceCount = 0;

  /// Actions - These are the callbacks we invoke as we parse various constructs
  /// in the file.
  Sema &Actions;

  // C++2a contextual keywords.
  mutable IdentifierInfo *Ident_import;
  mutable IdentifierInfo *Ident_module;

  /// Whether the '>' token acts as an operator or not. This will be
  /// true except when we are parsing an expression within a C++
  /// template argument list, where the '>' closes the template
  /// argument list.
  bool GreaterThanIsOperator;

  clang::DiagnosticsEngine &Diags;

  /// Factory object for creating ParsedAttr objects.
  clang::AttributeFactory AttrFactory;

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
      clang::Expr *TemplateName;
      clang::SourceLocation LessLoc;
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

    /// Add an expression that might have been intended to be a template name.
    /// In the case of ambiguity, we arbitrarily select the innermost such
    /// expression, for example in 'foo < bar < baz', 'bar' is the current
    /// candidate. No attempt is made to track that 'foo' is also a candidate
    /// for the case where we see a second suspicious '>' token.
    void add(Parser &P, clang::Expr *TemplateName, SourceLocation LessLoc,
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

  /// The location of the expression statement that is being parsed right now.
  /// Used to determine if an expression that is being parsed is a statement or
  /// just a regular sub-expression.
  clang::SourceLocation ExprStatementTokLoc;

  /// Flags describing a context in which we're parsing a statement.
  enum /*class*/ ParsedStmtContext {
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
  clang::StmtResult handleExprStmt(clang::ExprResult E,
                                   ParsedStmtContext StmtCtx);

public:
  Parser(Preprocessor &PP, Sema &Actions, bool SkipFunctionBodies);
  // ~Parser() override;

  const LangOptions &getLangOpts() const { return PP.getLangOpts(); }

  Scope *getCurScope() const { return Actions.getCurScope(); }

  // Type forwarding.  All of these are statically 'void*', but they may all be
  // different actual classes based on the actions in place.
  typedef clang::OpaquePtr<clang::DeclGroupRef> DeclGroupPtrTy;

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
  clang::SourceLocation ConsumeToken() {
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

  bool TryConsumeToken(tok::TokenKind Expected, clang::SourceLocation &Loc) {
    if (!TryConsumeToken(Expected))
      return false;
    Loc = PrevTokLocation;
    return true;
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

  /// isTokenSpecial - True if this token requires special consumption methods.
  bool isTokenSpecial() const {
    return isTokenStringLiteral() || isTokenParen() || isTokenBracket() ||
           isTokenBrace() /*||
           Tok.is(tok::code_completion) || Tok.isAnnotation()*/
        ;
  }

  SourceLocation ConsumeAnnotationToken() {
    assert(Tok.isAnnotation() && "wrong consume method");
    clang::SourceLocation Loc = Tok.getLocation();
    PrevTokLocation = Tok.getAnnotationEndLoc();
    PP.Lex(Tok);
    return Loc;
  }

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
  clang::SourceLocation ConsumeStringToken() {
    assert(isTokenStringLiteral() && "wrong consume method");
    PrevTokLocation = Tok.getLocation();
    PP.Lex(Tok);
    return PrevTokLocation;
  }

  /// Checks if the \p Level is valid for use in a fold expression.
  bool isFoldOperator(prec::Level Level) const;

  /// Checks if the \p Kind is a valid operator for fold expressions.
  bool isFoldOperator(tok::TokenKind Kind) const;

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

  struct ParsedAttributesWithRange : clang::ParsedAttributes {
    ParsedAttributesWithRange(clang::AttributeFactory &factory)
        : clang::ParsedAttributes(factory) {}

    void clean() {
      clang::ParsedAttributes::clear();
      Range = clang::SourceRange();
    }

    clang::SourceRange Range;
  };

  /// A SmallVector of statements, with stack size 32 (as that is the only one
  /// used.)
  typedef llvm::SmallVector<clang::Stmt *, 32> StmtVector;

  //===--------------------------------------------------------------------===//
  // C99 6.5: Expressions.

  /// TypeCastState - State whether an expression is or may be a type cast.
  enum TypeCastState { NotTypeCast = 0, MaybeTypeCast, IsTypeCast };

  clang::ExprResult ParseExpression(TypeCastState isTypeCast = NotTypeCast);

  //===--------------------------------------------------------------------===//
  // C++ 15: C++ Throw Expression
  clang::ExprResult ParseThrowExpression();
  // Expr that doesn't include commas.
  clang::ExprResult
  ParseAssignmentExpression(TypeCastState isTypeCast = NotTypeCast);

private:
  clang::ExprResult ParseRHSOfBinaryExpression(clang::ExprResult LHS,
                                               prec::Level MinPrec);

  /// Control what ParseCastExpression will parse.
  enum CastParseKind { AnyCastExpr = 0, UnaryExprOnly, PrimaryExprOnly };
  clang::ExprResult ParseCastExpression(CastParseKind ParseKind,
                                        bool isAddressOfOperand,
                                        bool &NotCastExpr,
                                        TypeCastState isTypeCast,
                                        bool isVectorLiteral = false,
                                        bool *NotPrimaryExpression = nullptr);
  clang::ExprResult ParseCastExpression(CastParseKind ParseKind,
                                        bool isAddressOfOperand = false,
                                        TypeCastState isTypeCast = NotTypeCast,
                                        bool isVectorLiteral = false,
                                        bool *NotPrimaryExpression = nullptr);

  /// Returns true if the next token cannot start an expression.
  bool isNotExpressionStart();

public:
  //===--------------------------------------------------------------------===//
  // C99 6.7.8: Initialization.

  /// ParseInitializer
  ///       initializer: [C99 6.7.8]
  ///         assignment-expression
  ///         '{' ...
  clang::ExprResult ParseInitializer() {
    if (Tok.isNot(tok::l_brace))
      return ParseAssignmentExpression();
    return ParseBraceInitializer();
  }
  bool MayBeDesignationStart();
  clang::ExprResult ParseBraceInitializer();
  clang::ExprResult ParseInitializerWithPotentialDesignator(
      llvm::function_ref<void(const clang::Designation &)> CodeCompleteCB);

  // Expr that doesn't include commas.
  clang::ExprResult
  ParseAssigmentExpression(TypeCastState isTypeCast = NotTypeCast);

  //===--------------------------------------------------------------------===//
  // C++ 6: Statements and Blocks
  clang::StmtResult ParseCXXTryBlock();

  //===--------------------------------------------------------------------===//
  // C99 6.8: Statements and Blocks.

  /// A SmallVector of statements, with stack size 32 (as that is the only one
  /// used.)
  typedef llvm::SmallVector<clang::Stmt *, 32> StmtVector;
  /// A SmallVector of expressions, with stack size 12 (the maximum used.)
  typedef llvm::SmallVector<clang::Expr *, 12> ExprVector;
  /// A SmallVector of types.
  // typedef llvm::SmallVector<ParsedType, 12> TypeVector;

  clang::StmtResult
  ParseStatement(clang::SourceLocation *TrailingElseLoc = nullptr,
                 ParsedStmtContext StmtCtx = ParsedStmtContext::SubStmt);

  clang::StmtResult
  ParseStatementOrDeclaration(StmtVector &Stmts, ParsedStmtContext StmtCtx,
                              clang::SourceLocation *TrailingElseLoc = nullptr);

  clang::StmtResult ParseStatementOrDeclarationAfterAttributes(
      StmtVector &Stmts, ParsedStmtContext StmtCtx,
      clang::SourceLocation *TrailingElseLoc, ParsedAttributesWithRange &Attrs);

  clang::StmtResult ParseExprStatement(ParsedStmtContext StmtCtx);

  clang::StmtResult ParseLabeledStatement(ParsedAttributesWithRange &attrs,
                                          ParsedStmtContext StmtCtx);

  clang::StmtResult
  ParseCaseStatement(ParsedStmtContext StmtCtx, bool MissingCase = false,
                     clang::ExprResult Expr = clang::ExprResult());

  clang::StmtResult ParseDefaultStatement(ParsedStmtContext StmtCtx);
  clang::StmtResult ParseIfStatement(clang::SourceLocation *TrailingElseLoc);
  clang::StmtResult
  ParseSwitchStatement(clang::SourceLocation *TrailingElseLoc);
  clang::StmtResult ParseWhileStatement(clang::SourceLocation *TrailingElseLoc);
  clang::StmtResult ParseDoStatement();
  clang::StmtResult ParseForStatement(clang::SourceLocation *TrailingElseLoc);
  clang::StmtResult ParseGotoStatement();
  clang::StmtResult ParseContinueStatement();
  clang::StmtResult ParseBreakStatement();
  clang::StmtResult ParseReturnStatement();

  bool isDeclarationSpecifier(bool DisambiguatingWithExpression = false);

  /// Return true if we know that we are definitely looking at a
  /// decl-specifier, and isn't part of an expression such as a function-style
  /// cast. Return false if it's no a decl-specifier, or we're not sure.
  bool isKnownToBeDeclarationSpecifier() {
    // if (getLangOpts().CPlusPlus)
    //   return isCXXDeclarationSpecifier() == TPResult::True;
    return isDeclarationSpecifier(true);
  }

  //===--------------------------------------------------------------------===//
  // Modules
  DeclGroupPtrTy ParseModuleDecl(bool IsFirstDecl);
  clang::Decl *ParseModuleImport(clang::SourceLocation AtLoc);
};

} // namespace latino

#endif