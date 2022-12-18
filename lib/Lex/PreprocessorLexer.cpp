#include "latino/Lex/PreprocessorLexer.h"
#include "latino/Basic/SourceManager.h"
#include "latino/Lex/Preprocessor.h"
#include "latino/Lex/Token.h"

#include <cassert>

using namespace latino;

PreprocessorLexer::PreprocessorLexer(Preprocessor *pp, FileID fid)
    : PP(pp), FID(fid) {
  // if (pp)
  // InitialNumSLocEntries = pp->getSourceManager().local_sloc_entry_size();
}