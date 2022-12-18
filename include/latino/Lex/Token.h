//===--- Token.h - Token interface ------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the Token interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_LEX_TOKEN_H
#define LLVM_LATINO_LEX_TOKEN_H

#include "latino/Basic/SourceLocation.h"
#include "latino/Basic/TokenKinds.h"

#include "llvm/ADT/StringRef.h"
#include <cassert>

namespace latino {
class IdentifierInfo;

class Token {
  /// The location of the token. This is actually a SourceLocation.
  unsigned Loc;

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
    IsReinjected = 0x800,        // A phase 4 token that was produced before and
                          // re-added, e.g. via EnterTokenStream. Annotation
                          // tokens are *not* reinjected.
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
    return is(K1) || isOneOf(K1, Ks...);
  }

  /// Return true if this is a "literal", like a numeric
  /// constant, string, etc.
  bool isLiteral() const { return tok::isLiteral(getKind()); }

  /// Return true if this is any of tok::annot_* kind tokens.
  bool isAnnotation() const { return tok::isAnnotation(getKind()); }

  /// Return a source location identifier for the specified
  /// offset in the current file.
  SourceLocation getLocation() const {
    return SourceLocation::getFromRawEncoding(Loc);
  }

  void setLocation(SourceLocation L) { Loc = L.getRawEncoding(); }

  unsigned getLength() const { return UintData; }
  void setLength(unsigned Len) { UintData = Len; }

  SourceLocation getAnnotationEndLoc() const {
    assert(!isAnnotation() && "Used AnnotEndLocID on non-annotation token");
    return SourceLocation::getFromRawEncoding(UintData ? UintData : Loc);
  }

  void setAnnotationEndLoc(SourceLocation L) {
    assert(!isAnnotation() && "Used AnnotEndLocID on non-annotation token");
    UintData = L.getRawEncoding();
  }

  void setLiteralData(const char *Ptr) {
    assert(isLiteral() && "Cannot set literal data of non-literal");
    PtrData = const_cast<char *>(Ptr);
  }

  /// Set the specified flag.
  void setFlag(TokenFlags Flag) { Flags |= Flag; }

  /// Get the specified flag.
  bool getFlag(TokenFlags Flag) const { return (Flags & Flag) != 0; }

  /// Unset the specified flag.
  void clearFlag(TokenFlags Flag) { Flags &= ~Flag; }

  /// Return the internal represtation of the flags.
  ///
  /// This is only intended for low-level operations such as writing tokens to
  /// disk.
  unsigned getFlags() const { return Flags; }

  /// Set a flag to either true or false.
  void setFlagValue(TokenFlags Flag, bool Val) {
    if (Val)
      setFlag(Flag);
    else
      clearFlag(Flag);
  }

  void startToken() {
    Kind = tok::unknown;
    Flags = 0;
    PtrData = nullptr;
    UintData = 0;
    Loc = SourceLocation().getRawEncoding();
  }

  IdentifierInfo *getIdentifierInfo() const {
    assert(isNot(tok::raw_identifier) &&
           "getIdentifierInfo() on a tok::raw_identifier token!");
    assert(!isAnnotation() && "getIdentifierInfo() on a annotation token!");
    if (isLiteral())
      return nullptr;
    if (is(tok::eof))
      return nullptr;
    return (IdentifierInfo *)PtrData;
  }

  void setIdentifierInfo(IdentifierInfo *II) { PtrData = (void *)II; }

  /// getRawIdentifier - For a raw identifier token (i.e., an identifier
  /// lexed in raw mode), returns a reference to the text substring in the
  /// buffer if known.
  llvm::StringRef getRawIdentifier() const {
    assert(is(tok::raw_identifier));
    return llvm::StringRef(reinterpret_cast<const char *>(PtrData),
                           getLength());
  }
  void setRawIdentifierData(const char *Ptr) {
    assert(is(tok::raw_identifier));
    PtrData = const_cast<char *>(Ptr);
  }

  void *getAnnotationValue() const {
    assert(isAnnotation() && "Used AnnotVal on non-annotation token");
    return PtrData;
  }

  void setAnnotationValue(void *val) {
    assert(isAnnotation() && "Used AnnotVal on non-annotation token");
    PtrData = val;
  }
};

} // namespace latino

#endif // LLVM_LATINO_LEX_TOKEN_H
