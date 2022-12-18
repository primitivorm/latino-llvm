#ifndef LLVM_LATINO_LEX_LEXER_H
#define LLVM_LATINO_LEX_LEXER_H

#include "latino/Basic/SourceLocation.h"

#include "latino/Basic/LangOptions.h"
#include "latino/Lex/PreprocessorLexer.h"
#include "latino/Lex/Token.h"

#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include <cassert>
#include <cstdint>
#include <string>

namespace llvm {
class MemoryBuffer;
}

namespace latino {

class SourceLocation;
class LangOptions;

class Lexer : public PreprocessorLexer {
  friend class Preprocessor;

  // void anchor() override;

  //===--------------------------------------------------------------------===//
  // Constant configuration values for this lexer.

  // Start of the buffer.
  const char *BufferStart;

  // End of the buffer.
  const char *BufferEnd;

  // Location for start of file.
  SourceLocation FileLoc;

  // LangOpts enabled by this language (cache).
  latino::LangOptions LangOpts;

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

  /// True if in raw mode.
  ///
  /// Raw mode disables interpretation of tokens and is a far faster mode to
  /// lex in than non-raw-mode.  This flag:
  ///  1. If EOF of the current lexer is found, the include stack isn't popped.
  ///  2. Identifier information is not looked up for identifier tokens.  As an
  ///     effect of this, implicit macro expansion is naturally disabled.
  ///  3. "#" tokens at the start of a line are treated as normal tokens, not
  ///     implicitly transformed by the lexer.
  ///  4. All diagnostic messages are disabled.
  ///  5. No callbacks are made into the preprocessor.
  ///
  /// Note that in raw mode that the PP pointer may be null.
  bool LexingRawMode = false;

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

  /// Diag - Forwarding function for diagnostics.  This translate a source
  /// position in the current buffer into a SourceLocation object for rendering.
  SourceLocation getSourceLocation(const char *Loc, unsigned TokLen = 1) const;

  /// Return the current location in the buffer.
  const char *getBufferLocation() const { return BufferPtr; }

  /// getCharAndSizeNoWarn - Like the getCharAndSize method, but does not ever
  /// emit a warning.
  static inline char getCharAndSizeNoWarn(const char *Ptr, unsigned &Size,
                                          const LangOptions &LangOpts) {
    // If this is not a trigraph and not a UCN or escaped newline, return
    // quickly.
    if (isObviouslySimpleCharacter(Ptr[0])) {
      Size = 1;
      return *Ptr;
    }
    Size = 0;
    return getCharAndSizeSlowNoWarn(Ptr, Size, LangOpts);
  }

  /// Given a location any where in a source buffer, find the location
  /// that corresponds to the beginning of the token in which the original
  /// source location lands.
  static SourceLocation GetBeginningOfToken(SourceLocation Loc,
                                            const SourceManager &SM,
                                            const LangOptions &LangOpts);

private:
  /// Lex - Return the next token in the file.  If this is the end of file, it
  /// return the tok::eof token.  This implicitly involves the preprocessor.
  bool Lex(Token &Result);

public:
  /// Checks whether new line pointed by Str is preceded by escape
  /// sequence.
  static bool isNewLineEscaped(const char *BufferStart, const char *Str);

  /// LexFromRawLexer - Lex a token from a designated raw lexer (one with no
  /// associated preprocessor object.  Return true if the 'next character to
  /// read' pointer points at the end of the lexer buffer, false otherwise.
  bool LexFromRawLexer(Token &Result) {
    assert(LexingRawMode && "Not already in ran mode!");
    Lex(Result);
    // Note that lexing to the end of the buffer doesn't implicitly delete the
    // lexer when in raw mode.
    return BufferPtr == BufferEnd;
  }

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
  static llvm::StringRef getSourceText(CharSourceRange Range,
                                       const SourceManager &SM,
                                       const LangOptions &LangOpts,
                                       bool *Invalid = nullptr);

  /// Finds the token that comes right after the given location.
  ///
  /// Returns the next token, or none if the location is inside a macro.
  static llvm::Optional<Token> findNextToken(SourceLocation Loc,
                                             const SourceManager &SM,
                                             const LangOptions &LangOpts);

  /// MeasureTokenLength - Relex the token at the specified location and return
  /// its length in bytes in the input file.  If the token needs cleaning (e.g.
  /// includes a trigraph or an escaped newline) then this count includes bytes
  /// that are part of that.
  static unsigned MeasureTokenLength(SourceLocation Loc,
                                     const SourceManager &SM,
                                     const LangOptions &LangOpts);

  /// Relex the token at the specified location.
  /// \returns true if there was a failure, false on success.
  static bool getRawToken(SourceLocation Loc, Token &Result,
                          const SourceManager &SM, const LangOptions &LangOpts,
                          bool IgnoreWhiteSpace = false);

  /// Computes the source location just past the end of the
  /// token at this source location.
  ///
  /// This routine can be used to produce a source location that
  /// points just past the end of the token referenced by \p Loc, and
  /// is generally used when a diagnostic needs to point just after a
  /// token where it expected something different that it received. If
  /// the returned source location would not be meaningful (e.g., if
  /// it points into a macro), this routine returns an invalid
  /// source location.
  ///
  /// \param Offset an offset from the end of the token, where the source
  /// location should refer to. The default offset (0) produces a source
  /// location pointing just past the end of the token; an offset of 1 produces
  /// a source location pointing to the last character in the token, etc.
  static SourceLocation getLocForEndOfToken(SourceLocation Loc, unsigned Offset,
                                            const SourceManager &SM,
                                            const LangOptions &LangOpts);

private:
  //===--------------------------------------------------------------------===//
  // Internal implementation interfaces.

  /// LexTokenInternal - Internal interface to lex a preprocessing token. Called
  /// by Lex.
  ///
  bool LexTokenInternal(Token &Result, bool TokAtPhysicalStartOfLine);

  /// FormTokenWithChars - When we lex a token, we have identified a span
  /// starting at BufferPtr, going to TokEnd that forms the token.  This method
  /// takes that range and assigns it to the token as its location and size.  In
  /// addition, since tokens cannot overlap, this also updates BufferPtr to be
  /// TokEnd.
  void FormTokenWithChars(Token &Result, const char *TokEnd,
                          tok::TokenKind Kind) {
    unsigned TokLen = TokEnd - BufferPtr;
    Result.setLength(TokLen);
    Result.setLocation(getSourceLocation(BufferPtr, TokLen));
    Result.setKind(Kind);
    BufferPtr = TokEnd;
  }

  //===--------------------------------------------------------------------===//
  // Other lexer functions.
  void SetByteOffset(unsigned Offset, bool StartOfLine);

  // Helper functions to lex the remainder of a token of the specific type.
  bool LexIdentifier(Token &Result, const char *CurPtr);
  bool LexNumericConstant(Token &Result, const char *CurPtr);
  bool LexStringLiteral(Token &Result, const char *CurPtr, tok::TokenKind Kind);
  bool LexEndOfFile(Token &Result, const char *CurPtr);
  bool SkipWhitespace(Token &Result, const char *CurPtr,
                      bool &TokAtPhysicalStartOfLine);
  bool SkipLineComment(Token &Result, const char *CurPtr,
                       bool &TokAtPhysicalStartOfLine);
  bool SkipBlockComment(Token &Result, const char *CurPtr,
                        bool &TokAtPhysicalStartOfLine);
  bool isHexaLiteral(const char *Start, const LangOptions &LangOpts);

  /// isObviouslySimpleCharacter - Return true if the specified character is
  /// obviously the same in translation phase 1 and translation phase 3.  This
  /// can return false for characters that end up being the same, but it will
  /// never return true for something that needs to be mapped.
  static bool isObviouslySimpleCharacter(char C) {
    return C != '?' && C != '\\';
  }

  /// getAndAdvanceChar - Read a single 'character' from the specified buffer,
  /// advance over it, and return it.  This is tricky in several cases.  Here we
  /// just handle the trivial case and fall-back to the non-inlined
  /// getCharAndSizeSlow method to handle the hard case.
  inline char getAndAdvanceChar(const char *&Ptr, Token &Tok) {
    // If this is not a trigraph and not a UCN or escaped newline, return
    // quickly.
    if (isObviouslySimpleCharacter(Ptr[0]))
      return *Ptr++;

    unsigned Size = 0;
    char C = getCharAndSizeSlow(Ptr, Size, &Tok);
    Ptr += Size;
    return C;
  }

  /// ConsumeChar - When a character (identified by getCharAndSize) is consumed
  /// and added to a given token, check to see if there are diagnostics that
  /// need to be emitted or flags that need to be set on the token.  If so, do
  /// it.
  const char *ConsumeChar(const char *Ptr, unsigned Size, Token &Tok) {
    // Normal case, we consumed exactly one token.  Just return it.
    if (Size == 1)
      return Ptr + Size;

    // Otherwise, re-lex the character with a current token, allowing
    // diagnostics to be emitted and flags to be set.
    Size = 0;
    getCharAndSizeSlow(Ptr, Size, &Tok);
    return Ptr + Size;
  }

  /// getCharAndSize - Peek a single 'character' from the specified buffer,
  /// get its size, and return it.  This is tricky in several cases.  Here we
  /// just handle the trivial case and fall-back to the non-inlined
  /// getCharAndSizeSlow method to handle the hard case.
  inline char getCharAndSize(const char *Ptr, unsigned &Size) {
    // If this is not a trigraph and not a UCN or escaped newline, return
    // quickly.
    if (isObviouslySimpleCharacter(Ptr[0])) {
      Size = 1;
      return *Ptr;
    }

    Size = 0;
    return getCharAndSizeSlow(Ptr, Size);
  }

  /// getCharAndSizeSlow - Handle the slow/uncommon case of the getCharAndSize
  /// method.
  char getCharAndSizeSlow(const char *Ptr, unsigned &Size,
                          Token *Tok = nullptr);

  /// getEscapedNewLineSize - Return the size of the specified escaped newline,
  /// or 0 if it is not an escaped newline. P[-1] is known to be a "\" on entry
  /// to this function.
  static unsigned getEscapedNewLineSize(const char *P);

  /// getCharAndSizeSlowNoWarn - Same as getCharAndSizeSlow, but never emits a
  /// diagnostic.
  static char getCharAndSizeSlowNoWarn(const char *Ptr, unsigned &Size,
                                       const LangOptions &LangOpts);
};
} // namespace latino

#endif // LLVM_LATINO_LEX_LEXER_H