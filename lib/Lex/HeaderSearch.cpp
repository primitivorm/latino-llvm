#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"

#include "latino/Basic/LangOptions.h"
#include "latino/Lex/HeaderSearch.h"

#include "latino/Lex/HeaderSearchOptions.h"
#include "clang/Basic/TargetInfo.h"

using namespace clang;
using namespace latino;

HeaderSearch::HeaderSearch(
    std::shared_ptr<HeaderSearchOptions> HSOpts,
    clang::SourceManager &SourceMgr, /* DiagnosticsEngine &Diags, */
    const LangOptions &LangOpts, const TargetInfo *Target)
    : HSOpts(std::move(HSOpts)), /* Diags(Diags), */
      FileMgr(SourceMgr.getFileManager()) {}