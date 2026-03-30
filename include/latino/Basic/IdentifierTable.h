//===- IdentifierTable.h - Hash table for identifier lookup -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Defines the latino::IdentifierInfo, latino::IdentifierTable, and
/// latino::Selector interfaces.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_BASIC_IDENTIFIERTABLE_H
#define LLVM_LATINO_BASIC_IDENTIFIERTABLE_H

#include "latino/Basic/LLVM.h"
#include "latino/Basic/TokenKinds.h"
#include "clang/Basic/LangOptions.h"
#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/PointerLikeTypeTraits.h"
#include "llvm/Support/type_traits.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>

namespace latino {

class DeclarationName;
class DeclarationNameTable;
class IdentifierInfo;
class LangOptions;
class SourceLocation;

/// A simple pair of identifier info and location.
using IdentifierLocPair = std::pair<IdentifierInfo *, SourceLocation>;

/// IdentifierInfo and other related classes are aligned to
/// 8 bytes so that DeclarationName can use the lower 3 bits
/// of a pointer to one of these classes.
enum { IdentifierInfoAlignment = 8 };

/// One of these records is kept for each identifier that
/// is lexed.  This contains information about whether the token was \#define'd,
/// is a language keyword, or if it is a front-end token of some sort (e.g. a
/// variable or function name).  The preprocessor keeps this information in a
/// set, and all tok::identifier tokens have a pointer to one of these.
/// It is aligned to 8 bytes because DeclarationName needs the lower 3 bits.
class alignas(IdentifierInfoAlignment) IdentifierInfo {
  friend class IdentifierTable;

  // Front-end token ID or tok::identifier.
  unsigned TokenID : 9;

  // True if there is a #define for this.
  unsigned HasMacro : 1;

  // True if there was a #define for this.
  unsigned HadMacro : 1;

  // True if the identifier is a language extension.
  unsigned IsExtension : 1;

  // True if the identifier is a keyword in a newer or proposed Standard.
  unsigned IsFutureCompatKeyword : 1;

  // True if the identifier is poisoned.
  unsigned IsPoisoned : 1;

  // True if the identifier is a C++ operator keyword.
  unsigned IsCPPOperatorKeyword : 1;

  // Internal bit set by the member function RecomputeNeedsHandleIdentifier.
  // See comment about RecomputeNeedsHandleIdentifier for more info.
  unsigned NeedsHandleIdentifier : 1;

  // True if the identifier was loaded (at least partially) from an AST file.
  unsigned IsFromAST : 1;

  // True if the identifier has changed from the definition
  // loaded from an AST file.
  unsigned ChangedAfterLoad : 1;

  // True if the identifier's frontend information has changed from the
  // definition loaded from an AST file.
  unsigned FEChangedAfterLoad : 1;

  // True if revertTokenIDToIdentifier was called.
  unsigned RevertedTokenID : 1;

  // True if there may be additional information about
  // this identifier stored externally.
  unsigned OutOfDate : 1;

  // True if this is the 'import' contextual keyword.
  unsigned IsModulesImport : 1;

  // True if this is a mangled OpenMP variant name.
  unsigned IsMangledOpenMPVariantName : 1;

  // 28 bits left in a 64-bit word.

  // Managed by the language front-end.
  void *FETokenInfo = nullptr;

  llvm::StringMapEntry<IdentifierInfo *> *Entry = nullptr;

  IdentifierInfo()
      : TokenID(tok::identifier), HasMacro(false), HadMacro(false),
        IsExtension(false), IsFutureCompatKeyword(false), IsPoisoned(false),
        IsCPPOperatorKeyword(false), NeedsHandleIdentifier(false),
        IsFromAST(false), ChangedAfterLoad(false), FEChangedAfterLoad(false),
        RevertedTokenID(false), OutOfDate(false), IsModulesImport(false),
        IsMangledOpenMPVariantName(false) {}

public:
  IdentifierInfo(const IdentifierInfo &) = delete;
  IdentifierInfo &operator=(const IdentifierInfo &) = delete;
  IdentifierInfo(IdentifierInfo &&) = delete;
  IdentifierInfo &operator=(IdentifierInfo &&) = delete;

  /// Return true if this is the identifier for the specified string.
  ///
  /// This is intended to be used for string literals only: II->isStr("foo").
  template <std::size_t StrLen> bool isStr(const char (&Str)[StrLen]) const {
    return getLength() == StrLen - 1 &&
           memcmp(getNameStart(), Str, StrLen - 1) == 0;
  }

  /// Return true if this is the identifier for the specified StringRef.
  bool isStr(llvm::StringRef Str) const {
    llvm::StringRef ThisStr(getNameStart(), getLength());
    return ThisStr == Str;
  }

  /// Return the beginning of the actual null-terminated string for this
  /// identifier.
  const char *getNameStart() const { return Entry->getKeyData(); }

  /// Efficiently return the length of this identifier info.
  unsigned getLength() const { return Entry->getKeyLength(); }

  /// Return the actual identifier string.
  StringRef getName() const { return StringRef(getNameStart(), getLength()); }

  /// Return true if this identifier is \#defined to some other value.
  /// \note The current definition may be in a module and not currently visible.
  bool hasMacroDefinition() const { return HasMacro; }
  void setHasMacroDefinition(bool Val) {
    if (HasMacro == Val)
      return;

    HasMacro = Val;
    if (Val) {
      NeedsHandleIdentifier = true;
      HadMacro = true;
    } else {
      RecomputeNeedsHandleIdentifier();
    }
  }
  /// Returns true if this identifier was \#defined to some value at any
  /// moment. In this case there should be an entry for the identifier in the
  /// macro history table in Preprocessor.
  bool hadMacroDefinition() const { return HadMacro; }

  /// If this is a source-language token (e.g. 'for'), this API
  /// can be used to cause the lexer to map identifiers to source-language
  /// tokens.
  tok::TokenKind getTokenID() const { return (tok::TokenKind)TokenID; }

  /// True if revertTokenIDToIdentifier() was called.
  bool hasRevertedTokenIDToIdentifier() const { return RevertedTokenID; }

  /// Revert TokenID to tok::identifier; used for GNU libstdc++ 4.2
  /// compatibility.
  ///
  /// TokenID is normally read-only but there are 2 instances where we revert it
  /// to tok::identifier for libstdc++ 4.2. Keep track of when this happens
  /// using this method so we can inform serialization about it.
  void revertTokenIDToIdentifier() {
    assert(TokenID != tok::identifier && "Already at tok::identifier");
    TokenID = tok::identifier;
    RevertedTokenID = true;
  }
  void revertIdentifierToTokenID(tok::TokenKind TK) {
    assert(TokenID == tok::identifier && "Should be at tok::identifier");
    TokenID = TK;
    RevertedTokenID = false;
  }

  /// Return the preprocessor keyword ID for this identifier.
  ///
  /// For example, "define" will return tok::pp_define.
  //tok::PPKeywordKind getPPKeywordID() const;

  //   /// Return a value indicating whether this is a builtin function.
  //   ///
  //   /// 0 is not-built-in. 1+ are specific builtin functions.
  //   unsigned getBuiltinID() const {
  //     if (ObjCOrBuiltinID >= tok::NUM_OBJC_KEYWORDS)
  //       return ObjCOrBuiltinID - tok::NUM_OBJC_KEYWORDS;
  //     else
  //       return 0;
  //   }

  //   void setBuiltinID(unsigned ID) {
  //     ObjCOrBuiltinID = ID + tok::NUM_OBJC_KEYWORDS;
  //     assert(ObjCOrBuiltinID - unsigned(tok::NUM_OBJC_KEYWORDS) == ID
  //            && "ID too large for field!");
  //   }

  /// get/setExtension - Initialize information about whether or not this
  /// language token is an extension.  This controls extension warnings, and is
  /// only valid if a custom token ID is set.
  bool isExtensionToken() const { return IsExtension; }
  void setIsExtensionToken(bool Val) {
    IsExtension = Val;
    if (Val)
      NeedsHandleIdentifier = true;
    else
      RecomputeNeedsHandleIdentifier();
  }

  /// is/setIsFutureCompatKeyword - Initialize information about whether or not
  /// this language token is a keyword in a newer or proposed Standard. This
  /// controls compatibility warnings, and is only true when not parsing the
  /// corresponding Standard. Once a compatibility problem has been diagnosed
  /// with this keyword, the flag will be cleared.
  bool isFutureCompatKeyword() const { return IsFutureCompatKeyword; }
  void setIsFutureCompatKeyword(bool Val) {
    IsFutureCompatKeyword = Val;
    if (Val)
      NeedsHandleIdentifier = true;
    else
      RecomputeNeedsHandleIdentifier();
  }

  /// setIsPoisoned - Mark this identifier as poisoned.  After poisoning, the
  /// Preprocessor will emit an error every time this token is used.
  void setIsPoisoned(bool Value = true) {
    IsPoisoned = Value;
    if (Value)
      NeedsHandleIdentifier = true;
    else
      RecomputeNeedsHandleIdentifier();
  }

  /// Return true if this token has been poisoned.
  bool isPoisoned() const { return IsPoisoned; }

  /// isCPlusPlusOperatorKeyword/setIsCPlusPlusOperatorKeyword controls whether
  /// this identifier is a C++ alternate representation of an operator.
  void setIsCPlusPlusOperatorKeyword(bool Val = true) {
    IsCPPOperatorKeyword = Val;
  }
  bool isCPlusPlusOperatorKeyword() const { return IsCPPOperatorKeyword; }

  /// Return true if this token is a keyword in the specified language.
  bool isKeyword(const clang::LangOptions &LangOpts) const;

  /// Return true if this token is a C++ keyword in the specified
  /// language.
  bool isCPlusPlusKeyword(const clang::LangOptions &LangOpts) const;

  /// Get and set FETokenInfo. The language front-end is allowed to associate
  /// arbitrary metadata with this token.
  void *getFETokenInfo() const { return FETokenInfo; }
  void setFETokenInfo(void *T) { FETokenInfo = T; }

  /// Return true if the Preprocessor::HandleIdentifier must be called
  /// on a token of this identifier.
  ///
  /// If this returns false, we know that HandleIdentifier will not affect
  /// the token.
  bool isHandleIdentifierCase() const { return NeedsHandleIdentifier; }

  /// Return true if the identifier in its current state was loaded
  /// from an AST file.
  bool isFromAST() const { return IsFromAST; }

  void setIsFromAST() { IsFromAST = true; }

  /// Determine whether this identifier has changed since it was loaded
  /// from an AST file.
  bool hasChangedSinceDeserialization() const { return ChangedAfterLoad; }

  /// Note that this identifier has changed since it was loaded from
  /// an AST file.
  void setChangedSinceDeserialization() { ChangedAfterLoad = true; }

  /// Determine whether the frontend token information for this
  /// identifier has changed since it was loaded from an AST file.
  bool hasFETokenInfoChangedSinceDeserialization() const {
    return FEChangedAfterLoad;
  }

  /// Note that the frontend token information for this identifier has
  /// changed since it was loaded from an AST file.
  void setFETokenInfoChangedSinceDeserialization() {
    FEChangedAfterLoad = true;
  }

  /// Determine whether the information for this identifier is out of
  /// date with respect to the external source.
  bool isOutOfDate() const { return OutOfDate; }

  /// Set whether the information for this identifier is out of
  /// date with respect to the external source.
  void setOutOfDate(bool OOD) {
    OutOfDate = OOD;
    if (OOD)
      NeedsHandleIdentifier = true;
    else
      RecomputeNeedsHandleIdentifier();
  }

  /// Determine whether this is the contextual keyword \c import.
  bool isModulesImport() const { return IsModulesImport; }

  /// Set whether this identifier is the contextual keyword \c import.
  void setModulesImport(bool I) {
    IsModulesImport = I;
    if (I)
      NeedsHandleIdentifier = true;
    else
      RecomputeNeedsHandleIdentifier();
  }

  /// Determine whether this is the mangled name of an OpenMP variant.
  bool isMangledOpenMPVariantName() const { return IsMangledOpenMPVariantName; }

  /// Set whether this is the mangled name of an OpenMP variant.
  void setMangledOpenMPVariantName(bool I) { IsMangledOpenMPVariantName = I; }

  /// Return true if this identifier is an editor placeholder.
  ///
  /// Editor placeholders are produced by the code-completion engine and are
  /// represented as characters between '<#' and '#>' in the source code. An
  /// example of auto-completed call with a placeholder parameter is shown
  /// below:
  /// \code
  ///   function(<#int x#>);
  /// \endcode
  bool isEditorPlaceholder() const {
    return getName().startswith("<#") && getName().endswith("#>");
  }

  /// Determine whether \p this is a name reserved for the implementation (C99
  /// 7.1.3, C++ [lib.global.names]).
  bool isReservedName(bool doubleUnderscoreOnly = false) const {
    if (getLength() < 2)
      return false;
    const char *Name = getNameStart();
    return Name[0] == '_' &&
           (Name[1] == '_' ||
            (Name[1] >= 'A' && Name[1] <= 'Z' && !doubleUnderscoreOnly));
  }

  /// Provide less than operator for lexicographical sorting.
  bool operator<(const IdentifierInfo &RHS) const {
    return getName() < RHS.getName();
  }

private:
  /// The Preprocessor::HandleIdentifier does several special (but rare)
  /// things to identifiers of various sorts.  For example, it changes the
  /// \c for keyword token from tok::identifier to tok::for.
  ///
  /// This method is very tied to the definition of HandleIdentifier.  Any
  /// change to it should be reflected here.
  void RecomputeNeedsHandleIdentifier() {
    NeedsHandleIdentifier = isPoisoned() || hasMacroDefinition() ||
                            isExtensionToken() || isFutureCompatKeyword() ||
                            isOutOfDate() || isModulesImport();
  }
};

/// An iterator that walks over all of the known identifiers
/// in the lookup table.
///
/// Since this iterator uses an abstract interface via virtual
/// functions, it uses an object-oriented interface rather than the
/// more standard C++ STL iterator interface. In this OO-style
/// iteration, the single function \c Next() provides dereference,
/// advance, and end-of-sequence checking in a single
/// operation. Subclasses of this iterator type will provide the
/// actual functionality.
class IdentifierIterator {
protected:
  IdentifierIterator() = default;

public:
  IdentifierIterator(const IdentifierIterator &) = delete;
  IdentifierIterator &operator=(const IdentifierIterator &) = delete;

  virtual ~IdentifierIterator();

  /// Retrieve the next string in the identifier table and
  /// advances the iterator for the following string.
  ///
  /// \returns The next string in the identifier table. If there is
  /// no such string, returns an empty \c StringRef.
  virtual StringRef Next() = 0;
};

/// Provides lookups to, and iteration over, IdentiferInfo objects.
class IdentifierInfoLookup {
public:
  virtual ~IdentifierInfoLookup();

  /// Return the IdentifierInfo for the specified named identifier.
  ///
  /// Unlike the version in IdentifierTable, this returns a pointer instead
  /// of a reference.  If the pointer is null then the IdentifierInfo cannot
  /// be found.
  virtual IdentifierInfo *get(StringRef Name) = 0;

  /// Retrieve an iterator into the set of all identifiers
  /// known to this identifier lookup source.
  ///
  /// This routine provides access to all of the identifiers known to
  /// the identifier lookup, allowing access to the contents of the
  /// identifiers without introducing the overhead of constructing
  /// IdentifierInfo objects for each.
  ///
  /// \returns A new iterator into the set of known identifiers. The
  /// caller is responsible for deleting this iterator.
  virtual IdentifierIterator *getIdentifiers();
};

} // namespace latino

#endif