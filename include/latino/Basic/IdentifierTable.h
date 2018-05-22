#ifndef LATINO_BASIC_IDENTIFIERTABLE_H
#define LATINO_BASIC_IDENTIFIERTABLE_H

#include "latino/Basic/LLVM.h"
#include "latino/Basic/TokenKinds.h"
#include "llvm/ADT/StringMap.h"

namespace latino {

class IdentifierInfo {
  friend class IdentifierTable;
  unsigned TokenID : 9;
  llvm::StringMapEntry<IdentifierInfo *> *Entry = nullptr;

public:
  IdentifierInfo();
  IdentifierInfo(const IdentifierInfo &) = delete;
  IdentifierInfo &operator=(const IdentifierInfo &) = delete;
  tok::TokenKind getTokenID() const { return (tok::TokenKind)TokenID; }
  const char *getNameStart() const {
    if (Entry)
      return Entry->getKeyData();
    using actualtype = std::pair<IdentifierInfo, const char *>;
    return ((const actualtype *)this)->second;
  }
  unsigned getLength() const {
    if (Entry)
      return Entry->getKeyLength();
    using actualtype = std::pair<IdentifierInfo, const char *>;
    const char *p = ((const actualtype *)this)->second - 2;
    return (((unsigned)p[0]) | (((unsigned)p[1]) << 8)) - 1;
  }
  StringRef getName() const { return StringRef(getNameStart(), getLength()); }
};

class IdentifierIterator {
protected:
  IdentifierIterator() = default;

public:
  IdentifierIterator(const IdentifierIterator &) = delete;
  IdentifierIterator &operator=(const IdentifierIterator &) = delete;
  virtual ~IdentifierIterator();
  virtual StringRef Next() = 0;
};

class IdentifierInfoLookup {
public:
  virtual ~IdentifierInfoLookup();
  virtual IdentifierInfo *get(StringRef Name) = 0;
  virtual IdentifierIterator *getIdentifiers();
};

class IdentifierTable {
  IdentifierInfoLookup *ExternalLookup;
  using HashTableTy = llvm::StringMap<IdentifierInfo *, llvm::BumpPtrAllocator>;
  HashTableTy HashTable;

public:
  IdentifierTable(IdentifierInfoLookup *externalLookup = nullptr);
  void setExternalIdentifierInfo(IdentifierInfoLookup *IILookup) {
    ExternalLookup = IILookup;
  }
  llvm::BumpPtrAllocator &getAllocator() { return HashTable.getAllocator(); }
  IdentifierInfo &get(StringRef Name) {
    auto &Entry = *HashTable.insert(std::make_pair(Name, nullptr)).first;
    IdentifierInfo *&II = Entry.second;
    if (II)
      return *II;
    if (ExternalLookup) {
      II = ExternalLookup->get(Name);
      if (II)
        return *II;
    }
    void *Mem = getAllocator().Allocate<IdentifierInfo>();
    II = new (Mem) IdentifierInfo();
    II->Entry = &Entry;
    return *II;
  }
};

} // namespace latino

#endif
