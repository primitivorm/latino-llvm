#include "latino/Basic/TokenKinds.h"

#include "llvm/Support/ErrorHandling.h"

using namespace latino;

static const char *const TokNames[] = {
#define TOK(X) #X,
#define KEYWORD(X, Y) #X,
#include "latino/Basic/TokenKinds.def"
    nullptr};

const char *tok::getTokenName(TokenKind Kind) {
  if (Kind < tok::NUM_TOKENS)
    return TokNames[Kind];
  llvm_unreachable("unknown TokenKind");
  return nullptr;
}