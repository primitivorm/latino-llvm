#include "latino/Basic/SourceManager.h"
#include "latino/Basic/SourceLocation.h"
//#include "latino/Lex/PTHManager.h"
#include "llvm/Support/MemoryBuffer.h"

#include <utility>

using namespace latino;
using namespace SrcMgr;

MemoryBuffer *ContentCache::getBuffer(const SourceManager &SM,
                                      SourceLocation Loc, bool *Invalid) const {
  if (Buffer.getPointer() || !ContentsEntry) {
    if (Invalid)
      *Invalid = isBufferInvalid();
    return Buffer.getPointer();
  }
  bool isVolatile = SM.userFilesAreVolatile(); // && !IsSystemFile;

  return Buffer.getPointer();
}

std::pair<FileID, unsigned> SourceManager::getDecomposedExpansionLocSlowCase(
    const SrcMgr::SLocEntry *E) const {
  FileID FID;
  SourceLocation Loc;
  unsigned Offset;
  do {
    Loc = E->getExpansion().getExpansionLocStart();
    FID = getFileID(Loc);
    E = &getSLocEntry(FID);
    Offset = Loc.getOffset() - E->getOffset();
  } while (!Loc.isFileID());
  return std::make_pair(FID, Offset);
}

FileID SourceManager::getFileIDSlow(unsigned SLocOffset) const {
  if (!SLocOffset)
    return FileID::get(0);
  if (SLocOffset < NextLocalOffset)
    return getFileIDLocal(SLocOffset);
  return getFileIDLoaded(SLocOffset);
}

FileID SourceManager::getFileIDLocal(unsigned SLocOffset) const {
  assert(SLocOffset < NextLocalOffset && "Bad function choice");
  const SrcMgr::SLocEntry *I;
  if (LastFileIDLookup.ID < 0 ||
      LocalSlocEntryTable[LastFileIDLookup.ID].getOffset() < SLocOffset) {
    I = LocalSlocEntryTable.end();
  } else {
    I = LocalSlocEntryTable.begin() + LastFileIDLookup.ID;
  }
  unsigned NumProbes = 0;
  while (true) {
    --I;
    if (I->getOffset() <= SLocOffset) {
      FileID Res = FileID::get(int(I - LocalSlocEntryTable.begin()));
      if (!I->isExpansion())
        LastFileIDLookup = Res;
      NumLinearScans += NumProbes + 1;
      return Res;
    }
    if (++NumProbes == 8)
      break;
  }
  unsigned GreaterIndex = I - LocalSlocEntryTable.begin();
  unsigned LessIndex = 0;
  NumProbes = 0;
  while (true) {
    bool Invalid = false;
    unsigned MiddleIndex = (GreaterIndex - LessIndex) / 2 + LessIndex;
    unsigned MidOffset = getLocalSLocEntry(MiddleIndex, &Invalid).getOffset();
    if (Invalid)
      return FileID::get(0);
    ++NumProbes;
    if (MidOffset > SLocOffset) {
      GreaterIndex = MiddleIndex;
      continue;
    }
    if (isOffsetInFileID(FileID::get(MiddleIndex), SLocOffset)) {
      FileID Res = FileID::get(MiddleIndex);
      if (!LocalSlocEntryTable[MiddleIndex].isExpansion())
        LastFileIDLookup = Res;
      NumBinaryProbes += NumProbes;
      return Res;
    }
    LessIndex = MiddleIndex;
  }
}

std::pair<FileID, unsigned>
SourceManager::getDecomposedSpellingLocSlowCase(const SrcMgr::SLocEntry *E,
                                                unsigned Offset) const {
  FileID FID;
  SourceLocation Loc;
  do {
    Loc = E->getExpansion().getSpellingLoc();
    Loc = Loc.getLocWithOffset(Offset);
    FID = getFileID(Loc);
    E = &getSLocEntry(FID);
    Offset = Loc.getOffset() - E->getOffset();
  } while (!Loc.isFileID());
  return std::make_pair(FID, Offset);
}

const SrcMgr::SLocEntry &SourceManager::loadSLocEntry(unsigned Index,
                                                      bool *Invalid) const {
  assert(!SLocEntryLoaded[Index]);
  if (ExternalSLocEntries->ReadSLocEntry(-(static_cast<int>(Index) + 2))) {
    if (Invalid)
      *Invalid = true;
    if (!SLocEntryLoaded[Index]) {
      LoadedSLocEntryTable[Index] = SLocEntry::get(
          0, FileInfo::get(SourceLocation(), getFakeContentCacheForRecovery(),
                           SrcMgr::Lat_User));
    }
  }
  return LoadedSLocEntryTable[Index];
}

const char *SourceManager::getCharacterData(SourceLocation SL,
                                            bool *Invalid) const {
  std::pair<FileID, unsigned> LocInfo = getDecomposedSpellingLoc(SL);
  bool CharDataInvalid = false;
  const SLocEntry &Entry = getSLocEntry(LocInfo.first, &CharDataInvalid);
  if (CharDataInvalid || !Entry.isFile()) {
    if (Invalid)
      *Invalid = true;
    return "<<<<INVALID BUFFER>>>>";
  }
  MemoryBuffer *Buffer = Entry.getFile().getContentCache()->getBuffer(
      *this, SourceLocation(), &CharDataInvalid);
  if (Invalid)
    *Invalid = CharDataInvalid;
  return Buffer->getBufferStart() + (CharDataInvalid ? 0 : LocInfo.second);
}

StringRef SourceManager::getBufferData(FileID FID, bool *Invalid) const {
  bool MyInvalid = false;
  const SLocEntry &SLoc = getSLocEntry(FID, &MyInvalid);
  if (!SLoc.isFile() || MyInvalid) {
    if (Invalid)
      *Invalid = true;
    return "<<<<< INVALID SOURCE LOCATION >>>>>";
  }
  MemoryBuffer *Buf = SLoc.getFile().getContentCache()->getBuffer(
      *this, SourceLocation(), &MyInvalid);
  if (Invalid)
    *Invalid = MyInvalid;
  if (MyInvalid)
    return "<<<<< INVALID SOURCE LOCATION >>>>>";
  return Buf->getBuffer();
}
