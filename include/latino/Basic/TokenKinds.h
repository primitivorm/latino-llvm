#ifndef LATINO_BASIC_TOKENKINDS_H
#define LATINO_BASIC_TOKENKINDS_H
#include "llvm/Support/Compiler.h"
namespace latino {
namespace tok {
enum TokenKind : unsigned short {
#define TOK(X) X,
#include "latino/Basic/TokenKinds.def"
  NUM_TOKENS
};

inline bool isLiteral(TokenKind K) {
  return K == tok::numeric_constant || K == tok::char_constant ||
         K == tok::string_literal;
}

inline bool isStringLiteral(TokenKind K) { return K == tok::string_literal; }

} // namespace tok
} // namespace latino
#endif /* LATINO_BASIC_TOKENKINDS_H */
