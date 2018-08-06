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

/// Provides a namespace for preprocessor keywords which start with a
/// '#' at the beginning of the line.
enum PPKeywordKind {
#define PPKEYWORD(X) pp_##X,
#include "latino/Basic/TokenKinds.def"
  NUM_PP_KEYWORDS
};

/// Provides a namespace for Objective-C keywords which start with
/// an '@'.
enum ObjCKeywordKind {
#define OBJC1_AT_KEYWORD(X) objc_##X,
#define OBJC2_AT_KEYWORD(X) objc_##X,
#include "latino/Basic/TokenKinds.def"
  NUM_OBJC_KEYWORDS
};

/// Defines the possible values of an on-off-switch (C99 6.10.6p2).
enum OnOffSwitch { OOS_ON, OOS_OFF, OOS_DEFAULT };

/// Determines the name of a token as used within the front end.
///
/// The name of a token will be an internal name (such as "l_square")
/// and should not be used as part of diagnostic messages.
const char *getTokenName(TokenKind Kind) LLVM_READONLY;

/// Determines the spelling of simple punctuation tokens like
/// '!' or '%', and returns NULL for literal and annotation tokens.
///
/// This routine only retrieves the "simple" spelling of the token,
/// and will not produce any alternative spellings (e.g., a
/// digraph). For the actual spelling of a given Token, use
/// Preprocessor::getSpelling().
const char *getPunctuatorSpelling(TokenKind Kind) LLVM_READNONE;

/// Determines the spelling of simple keyword and contextual keyword
/// tokens like 'int' and 'dynamic_cast'. Returns NULL for other token kinds.
const char *getKeywordSpelling(TokenKind Kind) LLVM_READNONE;

/// Return true if this is a raw identifier or an identifier kind.
inline bool isAnyIdentifier(TokenKind K) {
  return (K == tok::identifier) || (K == tok::raw_identifier);
}

/// Return true if this is a C or C++ string-literal (or
/// C++11 user-defined-string-literal) token.
inline bool isStringLiteral(TokenKind K) { return K == tok::string_literal; }

/// Return true if this is a "literal" kind, like a numeric
/// constant, string, etc.
inline bool isLiteral(TokenKind K) {
  return K == tok::numeric_constant || K == tok::char_constant ||
		 isStringLiteral(K) /*||
         K == tok::wide_char_constant || K == tok::utf8_char_constant ||
         K == tok::utf16_char_constant || K == tok::utf32_char_constant ||
	     K == tok::angle_string_literal*/;
}

/// Return true if this is any of tok::annot_* kinds.
inline bool isAnnotation(TokenKind K) {
#define ANNOTATION(NAME)                                                       \
  if (K == tok::annot_##NAME)                                                  \
    return true;
#include "latino/Basic/TokenKinds.def"
  return false;
}

} // namespace tok
} // namespace latino
#endif /* LATINO_BASIC_TOKENKINDS_H */
