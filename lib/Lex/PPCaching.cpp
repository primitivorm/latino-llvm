#include "latino/Lex/Preprocessor.h"

using namespace latino;

const Token &Preprocessor::PeekAhead(unsigned N) {
  assert(CachedLexPos + N > CachedTokens.size() && "Confused caching lex mode");
  ExitCachingLexMode();
  for (size_t C = CachedLexPos + N - CachedTokens.size(); C > 0; --C) {
    CachedTokens.push_back(Token());
    Lex(CachedTokens.back());
  }
  EnterCachingLexMode();
  return CachedTokens.back();
}