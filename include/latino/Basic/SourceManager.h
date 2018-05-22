#ifndef LATINO_BASIC_SOURCEMANAGER_H
#define LATINO_BASIC_SOURCEMANAGER_H

#include "latino/Basic/FileManager.h"
#include "latino/Basic/SourceLocation.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/PointerIntPair.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBuffer.h"

using namespace llvm;

namespace latino {

namespace SrcMgr {

enum CharacteristicKind { Lat_User };

class LLVM_ALIGNAS(8) ContentCache {
  enum CCFlags { InvalidFlag = 0x01, DoNotFreeFlag = 0x02 };

  const FileEntry *ContentsEntry;
  mutable PointerIntPair<MemoryBuffer *, 2> Buffer;

public:
  MemoryBuffer *getBuffer(const SourceManager &SM,
                          SourceLocation Loc = SourceLocation(),
                          bool *Invalid = nullptr) const;

  bool isBufferInvalid() const { return Buffer.getInt() & InvalidFlag; }
};

class FileInfo {
  unsigned IncludeLoc;
  unsigned NumCreatedFIDs : 31;
  unsigned HasLineDirectives : 1;
  llvm::PointerIntPair<const ContentCache *, 3, CharacteristicKind>
      ContentAndKind;

public:
  static FileInfo get(SourceLocation IL, const ContentCache *Con,
                      CharacteristicKind FileCharacter) {
    FileInfo X;
    X.IncludeLoc = IL.getRawEncoding();
    X.NumCreatedFIDs = 0;
    X.HasLineDirectives = false;
    X.ContentAndKind.setPointer(Con);
    X.ContentAndKind.setInt(FileCharacter);
    return X;
  }

  const ContentCache *getContentCache() const {
    return ContentAndKind.getPointer();
  }
}; /* FileInfo */

class ExpansionInfo {
  unsigned SpellingLoc;
  unsigned ExpansionLocStart, ExpansionLocEnd;

public:
  SourceLocation getSpellingLoc() const {
    return SourceLocation::getFromRawEncoding(SpellingLoc);
  }
  SourceLocation getExpansionLocStart() const {
    return SourceLocation::getFromRawEncoding(ExpansionLocStart);
  }
}; /* ExpansionInfo */

class SLocEntry {
  unsigned Offset : 31;
  unsigned IsExpansion : 1;
  union {
    FileInfo File;
    ExpansionInfo Expansion;
  };

public:
  SLocEntry() : Offset(), IsExpansion(), File() {}
  unsigned getOffset() const { return Offset; }
  bool isExpansion() const { return IsExpansion; }
  bool isFile() const { return !isExpansion(); }
  const FileInfo &getFile() const {
    assert(isFile() && "Not a file SLocEntry!");
    return File;
  }
  const ExpansionInfo &getExpansion() const {
    assert(isExpansion() && "Not a macro expansion SLocEntry!");
    return Expansion;
  }
  static SLocEntry get(unsigned Offset, const FileInfo &FI) {
    assert(!(Offset & (1 << 31)) && "Offset is too large");
    SLocEntry E;
    E.Offset = Offset;
    E.File = FI;
    return E;
  }
};
} // namespace SrcMgr

class ExternalSLocEntrySource {
public:
  virtual ~ExternalSLocEntrySource();
  virtual bool ReadSLocEntry(int ID) = 0;
  virtual std::pair<SourceLocation, StringRef> getModuleImportLoc(int ID) = 0;
};

class SourceManager : public RefCountedBase<SourceManager> {

  FileManager &FileMgr;
  bool UserFilesAreVolatile;
  SmallVector<SrcMgr::SLocEntry, 0> LocalSlocEntryTable;
  mutable SmallVector<SrcMgr::SLocEntry, 0> LoadedSLocEntryTable;
  llvm::BitVector SLocEntryLoaded;
  ExternalSLocEntrySource *ExternalSLocEntries = nullptr;
  mutable FileID LastFileIDLookup;
  unsigned NextLocalOffset;
  FileID MainFileID;
  mutable unsigned NumLinearScans = 0;
  mutable unsigned NumBinaryProbes = 0;

private:
  FileID getFileIDSlow(unsigned SLocOffset) const;
  FileID getFileIDLocal(unsigned SLocOffset) const;
  FileID getFileIDLoaded(unsigned SLocOffset) const;
  const SrcMgr::ContentCache *getFakeContentCacheForRecovery() const;
  const SrcMgr::SLocEntry &loadSLocEntry(unsigned Index, bool *Invalid) const;

  const SrcMgr::SLocEntry &getLoadedSLocEntry(unsigned Index,
                                              bool *Invalid = nullptr) const {
    assert(Index < LoadedSLocEntryTable.size() && "Invalid index");
    if (SLocEntryLoaded[Index])
      return LoadedSLocEntryTable[Index];
    return loadSLocEntry(Index, Invalid);
  }

  const SrcMgr::SLocEntry &
  getLoadedSLocEntryByID(int ID, bool *Invalid = nullptr) const {
    return getLoadedSLocEntry(static_cast<unsigned>(-ID - 2), Invalid);
  }

  const SrcMgr::SLocEntry &getLocalSLocEntry(unsigned Index,
                                             bool *Invalid = nullptr) const {
    assert(Index < LocalSlocEntryTable.size() && "Invalid index");
    return LocalSlocEntryTable[Index];
  }

  const SrcMgr::SLocEntry &getSLocEntryByID(int ID,
                                            bool *Invalid = nullptr) const {
    assert(ID != -1 && "Using FileID sentinel value");
    if (ID < 0)
      return getLoadedSLocEntryByID(ID, Invalid);
    return getLocalSLocEntry(static_cast<unsigned>(ID), Invalid);
  }

public:
  SourceManager(FileManager &FileMgr, bool UserFilesAreVolatile = false);
  explicit SourceManager(const SourceManager &) = delete;
  SourceManager &operator=(const SourceManager &) = delete;
  FileManager &getFileManager() const { return FileMgr; }
  StringRef getBufferData(FileID FID, bool *Invalid = nullptr) const;

  bool userFilesAreVolatile() const { return UserFilesAreVolatile; }

  FileID getMainFileID() const { return MainFileID; }

  bool isLoadedFileID(FileID FID) const {
    assert(FID.ID != -1 && "Using FileID sentinel value");
    return FID.ID < 0;
  }

  std::pair<FileID, unsigned>
  getDecomposedExpansionLocSlowCase(const SrcMgr::SLocEntry *E) const;

  std::pair<FileID, unsigned>
  getDecomposedSpellingLocSlowCase(const SrcMgr::SLocEntry *E,
                                   unsigned Offset) const;

  const char *getCharacterData(SourceLocation SL,
                               bool *Invalid = nullptr) const;

  std::pair<FileID, unsigned>
  getDecomposedSpellingLoc(SourceLocation Loc) const {
    FileID FID = getFileID(Loc);
    bool Invalid = false;
    const SrcMgr::SLocEntry *E = &getSLocEntry(FID, &Invalid);
    if (Invalid)
      return std::make_pair(FileID(), 0);
    unsigned Offset = Loc.getOffset() - E->getOffset();
    if (Loc.isFileID())
      return std::make_pair(FID, Offset);
    return getDecomposedSpellingLocSlowCase(E, Offset);
  }

  std::pair<FileID, unsigned> getDecomposedLoc(SourceLocation Loc) const {
    FileID FID = getFileID(Loc);
    bool Invalid = false;
    const SrcMgr::SLocEntry &E = getSLocEntry(FID, &Invalid);
    if (Invalid)
      return std::make_pair(FileID(), 0);
    return std::make_pair(FID, Loc.getOffset() - E.getOffset());
  }

  const SrcMgr::SLocEntry &getSLocEntry(FileID FID,
                                        bool *Invalid = nullptr) const {
    if (FID.ID == 0 || FID.ID == -1) {
      if (Invalid)
        *Invalid = true;
      return LocalSlocEntryTable[0];
    }
    return getSLocEntryByID(FID.ID, Invalid);
  }

  inline bool isOffsetInFileID(FileID FID, unsigned SLocOffset) const {
    const SrcMgr::SLocEntry &Entry = getSLocEntry(FID);
    if (SLocOffset < Entry.getOffset())
      return false;
    if (FID.ID == -2)
      return true;
    if (FID.ID + 1 == static_cast<int>(LocalSlocEntryTable.size()))
      return SLocOffset < NextLocalOffset;
    return SLocOffset < getSLocEntryByID(FID.ID + 1).getOffset();
  }

  FileID getFileID(SourceLocation SpellingLoc) const {
    unsigned SLocOffset = SpellingLoc.getOffset();
    if (isOffsetInFileID(LastFileIDLookup, SLocOffset))
      return LastFileIDLookup;
    return getFileIDSlow(SLocOffset);
  }

  SourceLocation getLocForStartOfFile(FileID FID) const {
    bool Invalid = false;
    const SrcMgr::SLocEntry &Entry = getSLocEntry(FID, &Invalid);
    if (Invalid)
      return SourceLocation();
    unsigned FileOffset = Entry.getOffset();
    return SourceLocation::getFileLoc(FileOffset);
  }
}; /* SourceManager */
} // namespace latino

#endif
