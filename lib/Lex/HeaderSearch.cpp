#include "latino/Lex/HeaderSearch.h"
#include "latino/Basic/FileManager.h"
#include "latino/Basic/SourceManager.h"
#include "latino/Basic/TargetInfo.h"
#include "latino/Lex/DirectoryLookup.h"
#include "latino/Lex/HeaderSearchOptions.h"

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

HeaderSearch::HeaderSearch(
    std::shared_ptr<HeaderSearchOptions> HSOpts,
    SourceManager &SourceMgr, /* DiagnosticsEngine &Diags, */
    const LangOptions &LangOpts, const TargetInfo *Target)
    : HSOpts(std::move(HSOpts)), /* Diags(Diags), */
      FileMgr(SourceMgr.getFileManager()) {}