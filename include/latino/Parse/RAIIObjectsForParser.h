#ifndef LLVM_LATINO_LIB_PARSE_RAIIOBJECTSFORPARSER_H
#define LLVM_LATINO_LIB_PARSE_RAIIOBJECTSFORPARSER_H

#include "latino/Parse/Parser.h"

namespace latino {

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
// class ParenBraceBracketBalancer {
//   Parser &P;
//   unsigned short ParenCount, BracketCount, BraceCount;

// public:
//   ParenBraceBracketBalancer(Parser &p)
//       : P(p), ParenCount(p.ParenCount), BracketCount(p.BracketCount),
//         BraceCount(p.BraceCount) {}

//   ~ParenBraceBracketBalancer() {
//     P.AngleBrackets.clear(P);
//     P.ParenCount = ParenCount;
//     P.BracketCount = BracketCount;
//     P.BraceCount = BraceCount;
//   }
// };

/// RAII class that helps handle the parsing of an open/close delimiter
/// pair, such as braces { ... } or parentheses ( ... ).
class BalancedDelimiterTracker : public GreaterThanIsOperatorScope {
  Parser &P;
  tok::TokenKind Kind, Close, FinalToken;
  clang::SourceLocation (Parser::*Consumer)();
  clang::SourceLocation LOpen, LClose;

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

  clang::SourceLocation getOpenLocation() const { return LOpen; }
  clang::SourceLocation getCloseLocation() const { return LClose; }
  clang::SourceRange getRange() const {
    return clang::SourceRange(LOpen, LClose);
  }

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