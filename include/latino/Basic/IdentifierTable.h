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
class IdentifierInfo;
class LangOptions;

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

  /// Return true if this token has been poisoned.
  bool isPoisoned() const { return IsPoisoned; }

  /// setIsPoisoned - Mark this identifier as poisoned.  After poisoning, the
  /// Preprocessor will emit an error every time this token is used.
  void setIsPoisoned(bool Value = true) {
    IsPoisoned = Value;
    if (Value)
      NeedsHandleIdentifier = true;
    else
      RecomputeNeedsHandleIdentifier();
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

  /// Populate the identifier table with info about the language keywords
  /// for the language specified by \p LangOpts.
  void AddKeywords(const LangOptions &LangOpts);
};

} // namespace latino

#endif
