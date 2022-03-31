#ifndef LLVM_LATINO_LEX_PREPROCESSORLEXER_H
#define LLVM_LATINO_LEX_PREPROCESSORLEXER_H

#include "clang/Basic/SourceLocation.h"

#include "latino/Lex/Token.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"

#include <cassert>

namespace latino {
class Preprocessor;

class PreprocessorLexer {

protected:
  friend class Preprocessor;

  // Preprocessor object controlling lexing.
  Preprocessor *PP = nullptr;

  /// The SourceManager FileID corresponding to the file being lexed.
  const clang::FileID FID;

  PreprocessorLexer() : FID() {}

  PreprocessorLexer(Preprocessor *pp, clang::FileID fid);
};
} // namespace latino

#endif // LLVM_LATINO_LEX_PREPROCESSORLEXER_H