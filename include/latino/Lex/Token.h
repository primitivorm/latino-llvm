#ifndef LATINO_LEX_TOKEN_H
#define LATINO_LEX_TOKEN_H

#include "latino/Basic/IdentifierTable.h"
#include "latino/Basic/LLVM.h"
#include "latino/Basic/SourceLocation.h"
#include "latino/Basic/TokenKinds.h"

namespace latino {

class Token {
  tok::TokenKind Kind;
  unsigned Loc;
  unsigned short Flags;
  unsigned UintData;
  void *PtrData;

public:
  enum TokenFlags {
    StartOfLine = 0x01,
    LeadingSpace = 0x02,
    DisabledExpand = 0x04,
    NeedsCleaning = 0x08,
    LeadingEmptyMacro = 0x10,
    HasUDSuffix = 0x20,
    HasUCN = 0x40,
    IgnoredComma = 0x80,
    StringfiedMacro = 0x100,
    CommaAfterElided = 0x200,
    IsEditorPlaceholder = 0x400,
  };

  void startToken() {
    Kind = tok::unknown;
    Flags = 0;
    PtrData = nullptr;
    UintData = 0;
    Loc = SourceLocation().getRawEncoding();
  }
  bool is(tok::TokenKind K) const { return Kind == K; }
  bool isLiteral() const { return tok::isLiteral(getKind()); }
  bool isNot(tok::TokenKind K) const { return Kind != K; }
  bool isOneOf(tok::TokenKind K1, tok::TokenKind K2) const {
    return is(K1) || is(K2);
  }
  SourceLocation getLocation() const {
    return SourceLocation::getFromRawEncoding(Loc);
  }
  tok::TokenKind getKind() const { return Kind; }
  void setKind(tok::TokenKind K) { Kind = K; }
  void setFlag(TokenFlags Flag) { Flags |= Flags; }
  bool getFlag(TokenFlags Flag) const { return (Flags & Flag) != 0; }
  void setIdentifierInfo(IdentifierInfo *II) { PtrData = (void *)II; }
  void clearFlag(TokenFlags Flag) { Flags &= ~Flags; }
  void setLocation(SourceLocation L) { Loc = L.getRawEncoding(); }
  void setLength(unsigned Len) { UintData = Len; }
  unsigned getLength() const { return UintData; }
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
