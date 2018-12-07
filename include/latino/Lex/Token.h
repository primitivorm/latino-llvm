//===----------------------------------------------------------------------===//
//
//  This file defines the Token interface.
//
//===----------------------------------------------------------------------===//

#ifndef LATINO_LEX_TOKEN_H
#define LATINO_LEX_TOKEN_H

#include "latino/Basic/IdentifierTable.h"
#include "latino/Basic/LLVM.h"
#include "latino/Basic/SourceLocation.h"
#include "latino/Basic/TokenKinds.h"

namespace latino {

/// Token - This structure provides full information about a lexed token.
/// It is not intended to be space efficient, it is intended to return as much
/// information as possible about each returned token.  This is expected to be
/// compressed into a smaller form if memory footprint is important.
///
/// The parser can create a special "annotation token" representing a stream of
/// tokens that were parsed and semantically resolved, e.g.: "foo::MyClass<int>"
/// can be represented by a single typename annotation token that carries
/// information about the SourceRange of the tokens and the type object.
class Token {
  /// The location of the token. This is actually a SourceLocation.
  unsigned Loc;

  /// The location of the token. This is actually a SourceLocation.

  // Conceptually these next two fields could be in a union.  However, this
  // causes gcc 4.2 to pessimize LexTokenInternal, a very performance critical
  // routine. Keeping as separate members with casts until a more beautiful fix
  // presents itself.

  /// UintData - This holds either the length of the token text, when
  /// a normal token, or the end of the SourceRange when an annotation
  /// token.
  unsigned UintData;

  /// PtrData - This is a union of four different pointer types, which depends
  /// on what type of token this is:
  ///  Identifiers, keywords, etc:
  ///    This is an IdentifierInfo*, which contains the uniqued identifier
  ///    spelling.
  ///  Literals:  isLiteral() returns true.
  ///    This is a pointer to the start of the token in a text buffer, which
  ///    may be dirty (have trigraphs / escaped newlines).
  ///  Annotations (resolved type names, C++ scopes, etc): isAnnotation().
  ///    This is a pointer to sema-specific data for the annotation token.
  ///  Eof:
  //     This is a pointer to a Decl.
  ///  Other:
  ///    This is null.
  void *PtrData;

  /// Kind - The actual flavor of token this is.
  tok::TokenKind Kind;

  /// Flags - Bits we track about this token, members of the TokenFlags enum.
  unsigned short Flags;

public:
  // Various flags set per token:
  enum TokenFlags {
    StartOfLine = 0x01,   // At start of line or only after whitespace
                          // (considering the line after macro expansion).
    LeadingSpace = 0x02,  // Whitespace exists before this token (considering
                          // whitespace after macro expansion).
    DisableExpand = 0x04, // This identifier may never be macro expanded.
    NeedsCleaning = 0x08, // Contained an escaped newline or trigraph.
    LeadingEmptyMacro = 0x10, // Empty macro exists before this token.
    HasUDSuffix = 0x20,  // This string or character literal has a ud-suffix.
    HasUCN = 0x40,       // This identifier contains a UCN.
    IgnoredComma = 0x80, // This comma is not a macro argument separator (MS).
    StringifiedInMacro = 0x100, // This string or character literal is formed by
                                // macro stringizing or charizing operator.
    CommaAfterElided = 0x200, // The comma following this token was elided (MS).
    IsEditorPlaceholder = 0x400, // This identifier is a placeholder.
  };

  tok::TokenKind getKind() const { return Kind; }
  void setKind(tok::TokenKind K) { Kind = K; }

  /// is/isNot - Predicates to check if this token is a specific kind, as in
  /// "if (Tok.is(tok::l_brace)) {...}".
  bool is(tok::TokenKind K) const { return Kind == K; }
  bool isNot(tok::TokenKind K) const { return Kind != K; }
  bool isOneOf(tok::TokenKind K1, tok::TokenKind K2) const {
    return is(K1) || is(K2);
  }
  template <typename... Ts>
  bool isOneOf(tok::TokenKind K1, tok::TokenKind K2, Ts... Ks) const {
    return is(K1) || isOneOf(K2, Ks...);
  }

  SourceLocation getLocation() const {
    return SourceLocation::getFromRawEncoding(Loc);
  }

  /// Return true if this is a "literal", like a numeric
  /// constant, string, etc.
  bool isLiteral() const { return tok::isLiteral(getKind()); }
  void setFlag(TokenFlags Flag) { Flags |= Flags; }
  bool getFlag(TokenFlags Flag) const { return (Flags & Flag) != 0; }
  void setIdentifierInfo(IdentifierInfo *II) { PtrData = (void *)II; }
  void clearFlag(TokenFlags Flag) { Flags &= ~Flags; }

  unsigned getLength() const { return UintData; }
  void setLocation(SourceLocation L) { Loc = L.getRawEncoding(); }
  void setLength(unsigned Len) { UintData = Len; }

  /// Reset all flags to cleared.
  void startToken() {
    Kind = tok::unknown;
    Flags = 0;
    PtrData = nullptr;
    UintData = 0;
    Loc = SourceLocation().getRawEncoding();
  }

  /// Return true if this token has trigraphs or escaped newlines in it.
  bool needsCleaning() const { return getFlag(NeedsCleaning); }
  IdentifierInfo *getIdentifierInfo() const {
    assert(isNot(tok::raw_identifier) &&
           "getIdentifierInfo() on a tok::raw_identifier token!");
    if (isLiteral())
      return nullptr;
    if (is(tok::eof))
      return nullptr;
    return (IdentifierInfo *)PtrData;
  }
  void setLiteralData(const char *Ptr) {
    assert(isLiteral() && "Cannot set literal data of non-literal");
    PtrData = const_cast<char *>(Ptr);
  }
  void setRawIdentifierData(const char *Ptr) {
    assert(is(tok::raw_identifier));
    PtrData = const_cast<char *>(Ptr);
  }
  StringRef getRawIdentifier() const {
    assert(is(tok::raw_identifier));
    return StringRef(reinterpret_cast<const char *>(PtrData), getLength());
  }
};

} // namespace latino

#endif /* LATINO_LEX_TOKEN_H */
