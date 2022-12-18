#include "latino/Lex/Preprocessor.h"

using namespace latino;

//===----------------------------------------------------------------------===//
// Methods for Entering and Callbacks for leaving various contexts
//===----------------------------------------------------------------------===//

/// EnterSourceFile - Add a source file to the top of the include stack and
/// start lexing tokens from it instead of the current buffer.
bool Preprocessor::EnterSourceFile(FileID FID, const DirectoryLookup *CurDir,
                                   SourceLocation Loc) {
  assert(!CurTokenLexer && "Cannot #include a file inside a macros!");
  ++NumEnteredSourceFiles;

  // if (MaxIncludeStackDepth < IncludeMacroStack.size())
  //   MaxIncludeStackDepth = IncludeMacroStack.size();

  // Get the MemoryBuffer for this FID, if it fails, we fail.
  bool Invalid = false;
  const llvm::MemoryBuffer *InputFile =
      getSourceManager().getBuffer(FID, Loc, &Invalid);

  if (Invalid) {
    SourceLocation FileStart = SourceMgr.getLocForStartOfFile(FID);
    // Diag(Loc, diag::err_pp_error_opening_file)
    //     << std::string(SourceMgr.getBufferName(FileStart)) << "";
    return true;
  }

  // if (isCodeCompletionEnabled() &&
  //     SourceMgr.getFileEntryForID(FID) == CodeCompletionFile) {
  //   CodeCompletionFileLoc = SourceMgr.getLocForStartOfFile(FID);
  //   CodeCompletionLoc =
  //       CodeCompletionFileLoc.getLocWithOffset(CodeCompletionOffset);
  // }

  EnterSourceFileWithLexer(new Lexer(FID, InputFile, *this), CurDir);
  return false;
}

/// EnterSourceFileWithLexer - Add a source file to the top of the include stack
///  and start lexing tokens from it instead of the current buffer.
void Preprocessor::EnterSourceFileWithLexer(Lexer *TheLexer,
                                            const DirectoryLookup *CurDir) {
  // Add the current lexer to the include stack.
  // if (CurPPLexer || CurTokenLexer)
  //   PushIncludeMacroStack();

  CurLexer.reset(TheLexer);
  CurPPLexer = TheLexer;
  CurDirLookup = CurDir;
  CurLexerSubmodule = nullptr;
  if (CurLexerKind != CLK_LexAfterModuleImport)
    CurLexerKind = CLK_Lexer;
}

/// RemoveTopOfLexerStack - Pop the current lexer/macro exp off the top of the
/// lexer stack.  This should only be used in situations where the current
/// state of the top-of-stack lexer is unknown.
void Preprocessor::RemoveTopOfLexerStack() {
  // assert(!IncludeMacroStack.empty() && "Ran out of stack entries to load");

  if (CurTokenLexer) {
    // Delete or cache the now-dead macro expander.
    if (NumCachedTokenLexers == TokenLexerCacheSize)
      CurTokenLexer.reset();
    else
      TokenLexerCache[NumCachedTokenLexers++] = std::move(CurTokenLexer);
  }
  PopIncludeMacroStack();
}