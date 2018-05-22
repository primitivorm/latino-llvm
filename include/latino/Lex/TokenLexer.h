#ifndef LATINO_LEX_TOKENLEXER_H
#define LATINO_LEX_TOKENLEXER_H

#include "latino/Basic/SourceLocation.h"
#include "latino/Lex/Token.h"
#include "llvm/ADT/ArrayRef.h"

namespace latino {
class Preprocessor;

class TokenLexer {
  Preprocessor &PP;
  const Token *Tokens;
  friend class Preprocessor;
  unsigned NumTokens;
  unsigned CurTokenIdx;
  SourceLocation ExpandLocStart, ExpandLocEnd;
  SourceLocation MacroStartSLocOffset;
  SourceLocation MacroDefStart;
  unsigned MacroDefLength;
  bool IsAtStartOfLine : 1;
  bool HasLeadingSpace : 1;
  bool NextTokGetsSapce : 1;
  bool OwnsTokens : 1;
  bool DisableMacroExpansion : 1;
  TokenLexer(const TokenLexer &) = delete;
  void operator=(const TokenLexer &) = delete;

private:
  void destroy();

public:
  TokenLexer(Token &Tok, SourceLocation ILEnd, Preprocessor &pp)
      : PP(pp), OwnsTokens(false) {
    Init(Tok, ILEnd);
  }
  void Init(Token &Tok, SourceLocation ILEnd);

  TokenLexer(const Token *TokArray, unsigned NumToks,
             bool disableMacroExpansion, bool ownsTokens, Preprocessor &pp)
      : PP(pp), OwnsTokens(false) {
    Init(TokArray, NumToks, DisableMacroExpansion, ownsTokens);
  }
  void Init(const Token *TokArray, unsigned NumToks, bool disableMacroExpansion,
            bool ownsTokens);
  ~TokenLexer() { destroy(); }
};
} // namespace latino
#endif
