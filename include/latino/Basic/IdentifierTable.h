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

  // 28 bits left in a 64-bit word.

  // Managed by the language front-end.
  void *FETokenInfo = nullptr;

  llvm::StringMapEntry<IdentifierInfo *> *Entry = nullptr;

  IdentifierInfo()
      : TokenID(tok::identifier), IsPoisoned(false),
        NeedsHandleIdentifier(false), IsFromAST(false), ChangedAfterLoad(false),
        FEChangedAfterLoad(false), RevertedTokenID(false), OutOfDate(false),
        IsModulesImport(false) {}

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
  bool isKeyword(const LangOptions &LangOpts) const;

  /// Return true if this token is a C++ keyword in the specified
  /// language.
  bool isCPlusPlusKeyword(const LangOptions &LangOpts) const;

  /// Get and set FETokenInfo. The language front-end is allowed to associate
  /// arbitrary metadata with this token.
  void *getFETokenInfo() const { return FETokenInfo; }
  void setFETokenInfo(void *T) { FETokenInfo = T; }

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
    NeedsHandleIdentifier = isPoisoned() || isOutOfDate() || isModulesImport();
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

/// Implements an efficient mapping from strings to IdentifierInfo nodes.
///
/// This has no other purpose, but this is an extremely performance-critical
/// piece of the code, as each occurrence of every identifier goes through
/// here when lexed.
class IdentifierTable {
  // Shark shows that using MallocAllocator is *much* slower than using this
  // BumpPtrAllocator!
  using HashTableTy = llvm::StringMap<IdentifierInfo *, llvm::BumpPtrAllocator>;
  HashTableTy HashTable;

  IdentifierInfoLookup *ExternalLookup;

public:
  /// Create the identifier table.
  explicit IdentifierTable(IdentifierInfoLookup *ExternalLookup = nullptr);

  /// Create the identifier table, populating it with info about the
  /// language keywords for the language specified by \p LangOpts.
  explicit IdentifierTable(const LangOptions &LangOpts,
                           IdentifierInfoLookup *ExternalLookup = nullptr);

  /// Set the external identifier lookup mechanism.
  void setExternalIdentifierLookup(IdentifierInfoLookup *IILookup) {
    ExternalLookup = IILookup;
  }

  /// Retrieve the external identifier lookup object, if any.
  IdentifierInfoLookup *getExternalIdentifierLookup() const {
    return ExternalLookup;
  }

  llvm::BumpPtrAllocator &getAllocator() { return HashTable.getAllocator(); }

  /// Return the identifier token info for the specified named
  /// identifier.
  IdentifierInfo &get(StringRef Name) {
    auto &Entry = *HashTable.insert(std::make_pair(Name, nullptr)).first;

    IdentifierInfo *&II = Entry.second;
    if (II)
      return *II;

    // No entry; if we have an external lookup, look there first.
    if (ExternalLookup) {
      II = ExternalLookup->get(Name);
      if (II)
        return *II;
    }

    // Lookups failed, make a new IdentifierInfo.
    void *Mem = getAllocator().Allocate<IdentifierInfo>();
    II = new (Mem) IdentifierInfo();

    // Make sure getName() knows how to find the IdentifierInfo
    // contents.
    II->Entry = &Entry;
    return *II;
  }

  IdentifierInfo &get(StringRef Name, tok::TokenKind TokenCode) {
    IdentifierInfo &II = get(Name);
    II.TokenID = TokenCode;
    assert(II.TokenID == (unsigned)TokenCode && "TokenCode too large");
    return II;
  }

  /// Gets an IdentifierInfo for the given name without consulting
  ///        external sources.
  ///
  /// This is a version of get() meant for external sources that want to
  /// introduce or modify an identifier. If they called get(), they would
  /// likely end up in a recursion.
  IdentifierInfo &getOwn(StringRef Name) {
    auto &Entry = *HashTable.insert(std::make_pair(Name, nullptr)).first;

    IdentifierInfo *&II = Entry.second;
    if (II)
      return *II;

    // Lookups failed, make a new IdentifierInfo.
    void *Mem = getAllocator().Allocate<IdentifierInfo>();
    II = new (Mem) IdentifierInfo();

    // Make sure getName() knows how to find the IdentifierInfo
    // contents.
    II->Entry = &Entry;

    // If this is the 'import' contextual keyword, mark it as such.
    if (Name.equals("import"))
      II->setModulesImport(true);

    return *II;
  }

  using iterator = HashTableTy::const_iterator;
  using const_iterator = HashTableTy::const_iterator;

  iterator begin() const { return HashTable.begin(); }
  iterator end() const { return HashTable.end(); }
  unsigned size() const { return HashTable.size(); }

  iterator find(StringRef Name) const { return HashTable.find(Name); }

  /// Print some statistics to stderr that indicate how well the
  /// hashing is doing.
  void PrintStats() const;

  /// Populate the identifier table with info about the language keywords
  /// for the language specified by \p LangOpts.
  void AddKeywords(const LangOptions &LangOpts);
};

namespace detail {

/// DeclarationNameExtra is used as a base of various uncommon special names.
/// This class is needed since DeclarationName has not enough space to store
/// the kind of every possible names. Therefore the kind of common names is
/// stored directly in DeclarationName, and the kind of uncommon names is
/// stored in DeclarationNameExtra. It is aligned to 8 bytes because
/// DeclarationName needs the lower 3 bits to store the kind of common names.
/// DeclarationNameExtra is tightly coupled to DeclarationName and any change
/// here is very likely to require changes in DeclarationName(Table).
class alignas(IdentifierInfoAlignment) DeclarationNameExtra {
  friend class latino::DeclarationName;
  friend class latino::DeclarationNameTable;

protected:
  /// The kind of "extra" information stored in the DeclarationName. See
  /// @c ExtraKindOrNumArgs for an explanation of how these enumerator values
  /// are used. Note that DeclarationName depends on the numerical values
  /// of the enumerators in this enum. See DeclarationName::StoredNameKind
  /// for more info.
  enum ExtraKind {
    CXXDeductionGuideName,
    CXXLiteralOperatorName,
    CXXUsingDirective //,
    // ObjCMultiArgSelector
  };

  /// ExtraKindOrNumArgs has one of the following meaning:
  ///  * The kind of an uncommon C++ special name. This DeclarationNameExtra
  ///    is in this case in fact either a CXXDeductionGuideNameExtra or
  ///    a CXXLiteralOperatorIdName.
  ///
  ///  * It may be also name common to C++ using-directives (CXXUsingDirective),
  ///
  ///  * Otherwise it is ObjCMultiArgSelector+NumArgs, where NumArgs is
  ///    the number of arguments in the Objective-C selector, in which
  ///    case the DeclarationNameExtra is also a MultiKeywordSelector.
  unsigned ExtraKindOrNumArgs;

  DeclarationNameExtra(ExtraKind Kind) : ExtraKindOrNumArgs(Kind) {}
  // DeclarationNameExtra(unsigned NumArgs)
  //     : ExtraKindOrNumArgs(ObjCMultiArgSelector + NumArgs) {}

  /// Return the corresponding ExtraKind.
  ExtraKind getKind() const {
    return static_cast<ExtraKind>(/*ExtraKindOrNumArgs >
                                          (unsigned)ObjCMultiArgSelector
                                      ? (unsigned)ObjCMultiArgSelector
                                      : */
                                  ExtraKindOrNumArgs);
  }

  /// Return the number of arguments in an ObjC selector. Only valid when this
  /// is indeed an ObjCMultiArgSelector.
  // unsigned getNumArgs() const {
  //   assert(ExtraKindOrNumArgs >= (unsigned)ObjCMultiArgSelector &&
  //          "getNumArgs called but this is not an ObjC selector!");
  //   return ExtraKindOrNumArgs - (unsigned)ObjCMultiArgSelector;
  // }
};

} // namespace detail

} // namespace latino

namespace llvm {

// Provide PointerLikeTypeTraits for IdentifierInfo pointers, which
// are not guaranteed to be 8-byte aligned.
template <> struct PointerLikeTypeTraits<latino::IdentifierInfo *> {
  static void *getAsVoidPointer(latino::IdentifierInfo *P) { return P; }

  static latino::IdentifierInfo *getFromVoidPointer(void *P) {
    return static_cast<latino::IdentifierInfo *>(P);
  }

  static constexpr int NumLowBitsAvailable = 1;
};

template <> struct PointerLikeTypeTraits<const latino::IdentifierInfo *> {
  static const void *getAsVoidPointer(const latino::IdentifierInfo *P) {
    return P;
  }

  static const latino::IdentifierInfo *getFromVoidPointer(const void *P) {
    return static_cast<const latino::IdentifierInfo *>(P);
  }

  static constexpr int NumLowBitsAvailable = 1;
};

} // namespace llvm

#endif
