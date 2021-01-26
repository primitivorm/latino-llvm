#ifndef LATINO_LLVM_LEX_LEXER_H
#define LATINO_LLVM_LEX_LEXER_H

#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/TokenKinds.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/PreprocessorLexer.h"
#include "clang/Lex/Token.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include <cassert>
#include <cstdint>
#include <string>

using namespace clang;

namespace latino {

class Lexer : public PreprocessorLexer {
  friend class Preprocessor;

  void anchor() override;

  //===--------------------------------------------------------------------===//
  // Constant configuration values for this lexer.

  // Start of the buffer.
  const char *BufferStart;

  // End of the buffer.
  const char *BufferEnd;

  // Location for start of file.
  SourceLocation FileLoc;

  // LangOpts enabled by this language (cache).
  LangOptions LangOpts;

  // True if lexer for _Pragma handling.
  bool Is_PragmaLexer;

  //===--------------------------------------------------------------------===//
  // Context-specific lexing flags set by the preprocessor.
  //

  /// ExtendedTokenMode - The lexer can optionally keep comments and whitespace
  /// and return them as tokens.  This is used for -C and -CC modes, and
  /// whitespace preservation can be useful for some clients that want to lex
  /// the file in raw mode and get every character from the file.
  ///
  /// When this is set to 2 it returns comments and whitespace.  When set to 1
  /// it returns comments, when it is set to 0 it returns normal tokens only.
  unsigned char ExtendedTokenMode;

  //===--------------------------------------------------------------------===//
  // Context that changes as the file is lexed.
  // NOTE: any state that mutates when in raw mode must have save/restore code
  // in Lexer::isNextPPTokenLParen.

  // BufferPtr - Current pointer into the buffer.  This is the next character
  // to be lexed.
  const char *BufferPtr;

  // IsAtStartOfLine - True if the next lexed token should get the "start of
  // line" flag set on it.
  bool IsAtStartOfLine;

  bool IsAtPhysicalStartOfLine;

  bool HasLeadingSpace;

  bool HasLeadingEmptyMacro;

  // CurrentConflictMarkerState - The kind of conflict marker we are handling.
  ConflictMarkerKind CurrentConflictMarkerState;

  void InitLexer(const char *BufStart, const char *BufPtr, const char *BufEnd);

public:
  /// Lexer constructor - Create a new lexer object for the specified buffer
  /// with the specified preprocessor managing the lexing process.  This lexer
  /// assumes that the associated file buffer and Preprocessor objects will
  /// outlive it, so it doesn't take ownership of either of them.
  Lexer(FileID FID, const llvm::MemoryBuffer *InputFile, Preprocessor &PP);

  /// Lexer constructor - Create a new raw lexer object.  This object is only
  /// suitable for calls to 'LexFromRawLexer'.  This lexer assumes that the
  /// text range will outlive it, so it doesn't take ownership of it.
  Lexer(SourceLocation FileLoc, const LangOptions &LangOpts,
        const char *BufStart, const char *BufPtr, const char *BufEnd);

  /// Lexer constructor - Create a new raw lexer object.  This object is only
  /// suitable for calls to 'LexFromRawLexer'.  This lexer assumes that the
  /// text range will outlive it, so it doesn't take ownership of it.
  Lexer(FileID FID, const llvm::MemoryBuffer *FromFile, const SourceManager &SM,
        const LangOptions &LangOpts);

  Lexer(const Lexer &) = delete;
  Lexer &operator=(const Lexer &) = delete;

  /// Checks whether new line pointed by Str is preceded by escape
  /// sequence.
  static bool isNewLineEscaped(const char *BufferStart, const char *Str);

  /// Given a location any where in a source buffer, find the location
  /// that corresponds to the beginning of the token in which the original
  /// source location lands.
  static SourceLocation GetBeginningOfToken(SourceLocation Loc,
                                            const SourceManager &SM,
                                            const LangOptions &LangOpts);

  /// Sets the extended token mode back to its initial value, according to the
  /// language options and preprocessor. This controls whether the lexer
  /// produces comment and whitespace tokens.
  ///
  /// This requires the lexer to have an associated preprocessor. A standalone
  /// lexer has nothing to reset to.
  void resetExtendedTokenMode();

  /// SetKeepWhitespaceMode - This method lets clients enable or disable
  /// whitespace retention mode.
  void SetKeepWhitespaceMode(bool Val) {
    assert((!Val || LexingRawMode || LangOpts.TraditionalCPP) &&
           "Can only retain whitespace in raw mode or -traditional-cpp");
    ExtendedTokenMode = Val ? 2 : 0;
  }

  /// isKeepWhitespaceMode - Return true if the lexer should return tokens for
  /// every character in the file, including whitespace and comments.  This
  /// should only be used in raw mode, as the preprocessor is not prepared to
  /// deal with the excess tokens.
  bool isKeepWhitespaceMode() const { return ExtendedTokenMode > 1; }

  /// SetCommentRetentionMode - Change the comment retention mode of the lexer
  /// to the specified mode.  This is really only useful when lexing in raw
  /// mode, because otherwise the lexer needs to manage this.
  void SetCommentRetentionState(bool Mode) {
    assert(!isKeepWhitespaceMode() &&
           "Can't play with comment retention state when retaining whitespace");
    ExtendedTokenMode = Mode ? 1 : 0;
  }

  /// Stringify - Convert the specified string into a C string by i) escaping
  /// '\\' and " characters and ii) replacing newline character(s) with "\\n".
  /// If Charify is true, this escapes the ' character instead of ".
  static std::string Stringify(StringRef Str, bool Charify = false);

  /// Stringify - Convert the specified string into a C string by i) escaping
  /// '\\' and " characters and ii) replacing newline character(s) with "\\n".
  static void Stringify(SmallVectorImpl<char> &Str);

  /// Finds the token that comes right after the given location.
  ///
  /// Returns the next token, or none if the location is inside a macro.
  static Optional<Token> findNextToken(SourceLocation Loc,
                                       const SourceManager &SM,
                                       const LangOptions &LangOpts);

  /// Accepts a range and returns a character range with file locations.
  ///
  /// Returns a null range if a part of the range resides inside a macro
  /// expansion or the range does not reside on the same FileID.
  ///
  /// This function is trying to deal with macros and return a range based on
  /// file locations. The cases where it can successfully handle macros are:
  ///
  /// -begin or end range lies at the start or end of a macro expansion, in
  ///  which case the location will be set to the expansion point, e.g:
  ///    \#define M 1 2
  ///    a M
  /// If you have a range [a, 2] (where 2 came from the macro), the function
  /// will return a range for "a M"
  /// if you have range [a, 1], the function will fail because the range
  /// overlaps with only a part of the macro
  ///
  /// -The macro is a function macro and the range can be mapped to the macro
  ///  arguments, e.g:
  ///    \#define M 1 2
  ///    \#define FM(x) x
  ///    FM(a b M)
  /// if you have range [b, 2], the function will return the file range "b M"
  /// inside the macro arguments.
  /// if you have range [a, 2], the function will return the file range
  /// "FM(a b M)" since the range includes all of the macro expansion.
  static CharSourceRange makeFileCharRange(CharSourceRange Range,
                                           const SourceManager &SM,
                                           const LangOptions &LangOpts);

  /// Returns a string for the source that the range encompasses.
  static StringRef getSourceText(CharSourceRange Range, const SourceManager &SM,
                                 const LangOptions &LangOpts,
                                 bool *Invalid = nullptr);
};

} // namespace latino

#endif // LATINO_LLVM_LEX_LEXER_H