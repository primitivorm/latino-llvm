#ifndef LLVM_LATINO_LIB_PARSE_RAIIOBJECTSFORPARSER_H
#define LLVM_LATINO_LIB_PARSE_RAIIOBJECTSFORPARSER_H

#include "latino/Sema/DelayedDiagnostic.h"

#include "latino/Parse/Parser.h"

namespace latino {

/// RAII object used to inform the actions that we're
/// currently parsing a declaration.  This is active when parsing a
/// variable's initializer, but not when parsing the body of a
/// class or function definition.
class ParsingDeclRAIIObject {
  Sema &Actions;
  sema::DelayedDiagnosticPool DiagnosticPool;
  Sema::ParsingDeclState State;
  bool Popped;

  ParsingDeclRAIIObject(const ParsingDeclRAIIObject &) = delete;
  void operator=(const ParsingDeclRAIIObject &) = delete;

public:
  enum NoParent_t { NoParent };
  ParsingDeclRAIIObject(Parser &P, NoParent_t _)
      : Actions(P.getActions()), DiagnosticPool(nullptr) {
    push();
  }

  /// Creates a RAII object whose pool is optionally parented by another.
  ParsingDeclRAIIObject(Parser &P,
                        const sema::DelayedDiagnosticPool *parentPool)
      : Actions(P.getActions()), DiagnosticPool(parentPool) {
    push();
  }

  /// Creates a RAII object and, optionally, initialize its
  /// diagnostics pool by stealing the diagnostics from another
  /// RAII object (which is assumed to be the current top pool).
  ParsingDeclRAIIObject(Parser &P, ParsingDeclRAIIObject *other)
      : Actions(P.getActions()),
        DiagnosticPool(other ? other->DiagnosticPool.getParent() : nullptr) {
    if (other) {
      DiagnosticPool.steal(other->DiagnosticPool);
      other->abort();
    }
    push();
  }

  ~ParsingDeclRAIIObject() { abort(); }

  sema::DelayedDiagnosticPool &getDelayedDiagnosticPool() {
    return DiagnosticPool;
  }
  const sema::DelayedDiagnosticPool &getDelayedDiagnosticPool() const {
    return DiagnosticPool;
  }

  /// Resets the RAII object for a new declaration.
  void reset() {
    abort();
    push();
  }

  /// Signals that the context was completed without an appropriate
  /// declaration being parsed.
  void abort() { pop(nullptr); }

  void complete(Decl *D) {
    assert(!Popped && "ParsingDeclaration has already been popped!");
    pop(D);
  }

  /// Unregister this object from Sema, but remember all the
  /// diagnostics that were emitted into it.
  void abortAndRemember() { pop(nullptr); }

private:
  void push() {
    State = Actions.PushParsingDeclaration(DiagnosticPool);
    Popped = false;
  }

  void pop(Decl *D) {
    if (!Popped) {
      Actions.PopParsingDeclaration(State, D);
      Popped = true;
    }
  }
};

/// A class for parsing a DeclSpec.
class ParsingDeclSpec : public DeclSpec {
  ParsingDeclRAIIObject ParsingRAII;

public:
  ParsingDeclSpec(Parser &P)
      : DeclSpec(/* P.getAttrFactory()*/),
        ParsingRAII(P, ParsingDeclRAIIObject::NoParent) {}
  ParsingDeclSpec(Parser &P, ParsingDeclRAIIObject *RAII)
      : DeclSpec(/* P.getAttrFactory()*/), ParsingRAII(P, RAII) {}

  const sema::DelayedDiagnosticPool &getDelayedDiagnosticPool() const {
    return ParsingRAII.getDelayedDiagnosticPool();
  }

  void complete(Decl *D) { ParsingRAII.complete(D); }

  void abort() { ParsingRAII.abort(); }
};

/// A class for parsing a declarator.
class ParsingDeclarator : public Declarator {
  ParsingDeclRAIIObject ParsingRAII;

public:
  ParsingDeclarator(Parser &P, const ParsingDeclSpec &DS, DeclaratorContext C)
      : Declarator(DS, C), ParsingRAII(P, &DS.getDelayedDiagnosticPool()) {}

  const ParsingDeclSpec &getDeclSpec() const {
    return static_cast<const ParsingDeclSpec &>(Declarator::getDeclSpec());
  }

  ParsingDeclSpec &getMutableDeclSpec() const {
    return const_cast<ParsingDeclSpec &>(getDeclSpec());
  }

  void clear() {
    Declarator::clear();
    ParsingRAII.reset();
  }

  void complete(Decl *D) { ParsingRAII.complete(D); }
};

/// A class for parsing a field declarator.
class ParsingFieldDeclarator : public FieldDeclarator {
  ParsingDeclRAIIObject ParsingRAII;

public:
  ParsingFieldDeclarator(Parser &P, const ParsingDeclSpec &DS)
      : FieldDeclarator(DS), ParsingRAII(P, &DS.getDelayedDiagnosticPool()) {}

  const ParsingDeclSpec &getDeclSpec() const {
    return static_cast<const ParsingDeclSpec &>(D.getDeclSpec());
  }

  ParsingDeclSpec &getMutableDeclSpec() const {
    return const_cast<ParsingDeclSpec &>(getDeclSpec());
  }

  void complete(Decl *D) { ParsingRAII.complete(D); }
};

/// ColonProtectionRAIIObject - This sets the Parser::ColonIsSacred bool and
/// restores it when destroyed.  This says that "foo:" should not be
/// considered a possible typo for "foo::" for error recovery purposes.
class ColonProtectionRAIIObject {
  Parser &P;
  bool OldVal;

public:
  ColonProtectionRAIIObject(Parser &p, bool Value = true)
      : P(p), OldVal(P.ColonIsSacred) {
    P.ColonIsSacred = Value;
  }

  /// restore - This can be used to restore the state early, before the dtor
  /// is run.
  void restore() { P.ColonIsSacred = OldVal; }

  ~ColonProtectionRAIIObject() { restore(); }
};

/// RAII object that makes '>' behave either as an operator
/// or as the closing angle bracket for a template argument list.
class GreaterThanIsOperatorScope {
  bool &GreaterThanIsOperator;
  bool OldGreaterThanIsOperator;

public:
  GreaterThanIsOperatorScope(bool &GTIO, bool Val)
      : GreaterThanIsOperator(GTIO), OldGreaterThanIsOperator(GTIO) {
    GreaterThanIsOperator = Val;
  }

  ~GreaterThanIsOperatorScope() {
    GreaterThanIsOperator = OldGreaterThanIsOperator;
  }
};

/// RAII object that makes sure paren/bracket/brace count is correct
/// after declaration/statement parsing, even when there's a parsing error.
class ParenBraceBracketBalancer {
  Parser &P;
  unsigned short ParenCount, BracketCount, BraceCount;

public:
  ParenBraceBracketBalancer(Parser &p)
      : P(p), ParenCount(p.ParenCount), BracketCount(p.BracketCount),
        BraceCount(p.BraceCount) {}

  ~ParenBraceBracketBalancer() {
    P.AngleBrackets.clear(P);
    P.ParenCount = ParenCount;
    P.BracketCount = BracketCount;
    P.BraceCount = BraceCount;
  }
};

/// RAII class that helps handle the parsing of an open/close delimiter
/// pair, such as braces { ... } or parentheses ( ... ).
class BalancedDelimiterTracker : public GreaterThanIsOperatorScope {
  Parser &P;
  tok::TokenKind Kind, Close, FinalToken;
  SourceLocation (Parser::*Consumer)();
  SourceLocation LOpen, LClose;

  unsigned short &getDepth() {
    switch (Kind) {
    case tok::l_brace:
      return P.BraceCount;
    case tok::l_square:
      return P.BracketCount;
    case tok::l_paren:
      return P.ParenCount;
    default:
      llvm_unreachable("Wrong token kind");
    }
  }

  bool diagnoseOverflow();
  bool diagnoseMissingClose();

public:
  BalancedDelimiterTracker(Parser &p, tok::TokenKind k,
                           tok::TokenKind FinalToken = tok::semi)
      : GreaterThanIsOperatorScope(p.GreaterThanIsOperator, true), P(p),
        Kind(k), FinalToken(FinalToken) {
    switch (Kind) {
    default:
      llvm_unreachable("Unexpected balanced token");
    case tok::l_brace:
      Close = tok::r_brace;
      Consumer = &Parser::ConsumeBrace;
      break;
    case tok::l_paren:
      Close = tok::r_paren;
      Consumer = &Parser::ConsumeParen;
      break;

    case tok::l_square:
      Close = tok::r_square;
      Consumer = &Parser::ConsumeBracket;
      break;
    }
  }

  SourceLocation getOpenLocation() const { return LOpen; }
  SourceLocation getCloseLocation() const { return LClose; }
  SourceRange getRange() const { return SourceRange(LOpen, LClose); }

  bool consumeOpen() {
    if (!P.Tok.is(Kind))
      return true;

    if (getDepth() < P.getLangOpts().BracketDepth) {
      LOpen = (P.*Consumer)();
      return false;
    }

    return diagnoseOverflow();
  }

  bool expectAndConsume(/*unsigned DiagID = diag::err_expected,*/
                        const char *Msg = "",
                        tok::TokenKind SkipToTok = tok::unknown);

  bool consumeClose() {
    if (P.Tok.is(Close)) {
      LClose = (P.*Consumer)();
      return false;
    } else if (P.Tok.is(tok::semi) && P.NextToken().is(Close)) {
      SourceLocation SemiLoc = P.ConsumeToken();
      // P.Diag(SemiLoc, diag::err_unexpected_semi)
      //     << Close << FixItHint::CreateRemoval(SourceRange(SemiLoc,
      //     SemiLoc));
      LClose = (P.*Consumer)();
      return false;
    }

    return diagnoseMissingClose();
  }
  void skipToEnd();
};

} // namespace latino

#endif