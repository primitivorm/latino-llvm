#include "latino/Lex/HeaderSearch.h"

#include "clang/Basic/FileManager.h"

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/Capacity.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/VirtualFileSystem.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>
#include <system_error>
#include <utility>

using namespace latino;


//===----------------------------------------------------------------------===//
// File Info Management.
//===----------------------------------------------------------------------===//

/// Merge the header file info provided by \p OtherHFI into the current
/// header file info (\p HFI)
static void mergeHeaderFileInfo(HeaderFileInfo &HFI,
                                const HeaderFileInfo &OtherHFI) {
  assert(OtherHFI.External && "expected to merge external HFI");

  HFI.isImport |= OtherHFI.isImport;
  HFI.isPragmaOnce |= OtherHFI.isPragmaOnce;
  HFI.isModuleHeader |= OtherHFI.isModuleHeader;
  HFI.NumIncludes += OtherHFI.NumIncludes;

  if (!HFI.ControllingMacro && !HFI.ControllingMacroID) {
    HFI.ControllingMacro = OtherHFI.ControllingMacro;
    HFI.ControllingMacroID = OtherHFI.ControllingMacroID;
  }

  HFI.DirInfo = OtherHFI.DirInfo;
  HFI.External = (!HFI.IsValid || HFI.External);
  HFI.IsValid = true;
  HFI.IndexHeaderMapHeader = OtherHFI.IndexHeaderMapHeader;

  if (HFI.Framework.empty())
    HFI.Framework = OtherHFI.Framework;
}

/// getFileInfo - Return the HeaderFileInfo structure for the specified
/// FileEntry.
HeaderFileInfo &HeaderSearch::getFileInfo(const clang::FileEntry *FE) {
  if (FE->getUID() >= FileInfo.size())
    FileInfo.resize(FE->getUID() + 1);

  HeaderFileInfo *HFI = &FileInfo[FE->getUID()];
  // FIXME: Use a generation count to check whether this is really up to date.
  if (ExternalSource && !HFI->Resolved) {
    HFI->Resolved = true;
    auto ExternalHFI = ExternalSource->GetHeaderFileInfo(FE);

    HFI = &FileInfo[FE->getUID()];
    if (ExternalHFI.External)
      mergeHeaderFileInfo(*HFI, ExternalHFI);
  }

  HFI->IsValid = true;
  // We have local information about this header file, so it's no longer
  // strictly external.
  HFI->External = false;
  return *HFI;
}

