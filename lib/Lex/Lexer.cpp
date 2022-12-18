#include "latino/Lex/Lexer.h"
#include "latino/Basic/CharInfo.h"
#include "latino/Basic/SourceLocation.h"
#include "latino/Basic/SourceManager.h"
#include "latino/Lex/Preprocessor.h"
#include "latino/Lex/Token.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/MemoryBuffer.h"

#include <cassert>

using namespace latino;

void Lexer::InitLexer(const char *BufStart, const char *BufPtr,
                      const char *BufEnd) {
  BufferStart = BufStart;
  BufferPtr = BufPtr;
  BufferEnd = BufEnd;

  assert(BufEnd[0] == 0 &&
         "We assume that the input buffer has a null character at the end"
         " to simplify lexing!");

  // Check whether we have a BOM in the beginning of the buffer. If yes - act
  // accordingly. Right now we support only UTF-8 with and without BOM, so, just
  // skip the UTF-8 BOM if it's present.
  if (BufferStart == BufferPtr) {
    // Determine the size if the BOM.
    llvm::StringRef Buf(BufferStart, BufferEnd - BufferStart);
    size_t BOMLength = llvm::StringSwitch<size_t>(Buf)
                           .StartsWith("\xEF\xBB\xBF", 3) // UTF-8 BOM
                           .Default(0);
    // Skip the BOM.
    BufferPtr += BOMLength;
  }

  // Start of the file is a start of line.
  IsAtStartOfLine = true;
  IsAtPhysicalStartOfLine = true;

  HasLeadingSpace = false;

  // We are not in raw mode.  Raw mode disables diagnostics and interpretation
  // of tokens (e.g. identifiers, thus disabling macro expansion).  It is used
  // to quickly lex the tokens of the buffer, e.g. when handling a "#if 0" block
  // or otherwise skipping over tokens.
  LexingRawMode = false;
}

/// Lexer constructor - Create a new lexer object for the specified buffer
/// with the specified preprocessor managing the lexing process.  This lexer
/// assumes that the associated file buffer and Preprocessor objects will
/// outlive it, so it doesn't take ownership of either of them.
Lexer::Lexer(FileID FID, const llvm::MemoryBuffer *InputFile, Preprocessor &PP)
    : PreprocessorLexer(&PP, FID),
      FileLoc(PP.getSourceManager().getLocForStartOfFile(FID)),
      LangOpts(PP.getLangOpts()) {
  InitLexer(InputFile->getBufferStart(), InputFile->getBufferStart(),
            InputFile->getBufferEnd());
}

/// Lexer constructor - Create a new raw lexer object.  This object is only
/// suitable for calls to 'LexFromRawLexer'.  This lexer assumes that the text
/// range will outlive it, so it doesn't take ownership of it.
Lexer::Lexer(SourceLocation fileLoc, const LangOptions &langOpts,
             const char *BufStart, const char *BufPtr, const char *BufEnd)
    : FileLoc(fileLoc) {
  InitLexer(BufStart, BufPtr, BufEnd);

  // We *are* in raw mode.
  LexingRawMode = true;
}

/// Lexer constructor - Create a new raw lexer object.  This object is only
/// suitable for calls to 'LexFromRawLexer'.  This lexer assumes that the text
/// range will outlive it, so it doesn't take ownership of it.
Lexer::Lexer(FileID FID, const llvm::MemoryBuffer *FromFile,
             const SourceManager &SM, const LangOptions &langOpts)
    : Lexer(SM.getLocForStartOfFile(FID), langOpts, FromFile->getBufferStart(),
            FromFile->getBufferStart(), FromFile->getBufferEnd()) {}

//===----------------------------------------------------------------------===//
// Helper methods for lexing.
//===----------------------------------------------------------------------===//
/// Routine that indiscriminately sets the offset into the source file.
void Lexer::SetByteOffset(unsigned Offset, bool StartOfLine) {
  BufferPtr = BufferStart + Offset;
  if (BufferPtr > BufferEnd)
    BufferPtr = BufferEnd;

  // FIXME: What exactly does the StartOfLine bit mean?  There are two
  // possible meanings for the "start" of the line: the first token on the
  // unexpanded line, or the first token on the expanded line.
  IsAtStartOfLine = StartOfLine;
  IsAtPhysicalStartOfLine = StartOfLine;
}

bool Lexer::isNewLineEscaped(const char *BufferStart, const char *Str) {
  assert(isVerticalWhitespace(Str[0]));
  if (Str - 1 < BufferStart)
    return false;

  if ((Str[0] == '\n' && Str[-1] == '\r') ||
      (Str[0] == '\r' && Str[-1] == '\n')) {
    if (Str - 2 < BufferStart)
      return false;
    --Str;
  }
  --Str;

  // Rewind to first non-space character:
  while (Str > BufferStart && isHorizontalWhitespace(*Str))
    --Str;

  return *Str == '\\';
}

static CharSourceRange makeRangeFromFileLocs(CharSourceRange Range,
                                             const SourceManager &SM,
                                             const LangOptions &LangOpts) {
  SourceLocation Begin = Range.getBegin();
  SourceLocation End = Range.getEnd();
  assert(Begin.isFileID() && End.isFileID());
  if (Range.isTokenRange()) {
    End = Lexer::getLocForEndOfToken(End, 0, SM, LangOpts);
    if (End.isInvalid())
      return {};
  }

  // Break down the source locations.
  FileID FID;
  unsigned BeginOffs;
  std::tie(FID, BeginOffs) = SM.getDecomposedLoc(Begin);
  if (FID.isInvalid())
    return {};

  unsigned EndOffs;
  if (!SM.isInFileID(End, FID, &EndOffs) || BeginOffs > EndOffs)
    return {};
  return CharSourceRange::getCharRange(Begin, End);
}

CharSourceRange Lexer::makeFileCharRange(CharSourceRange Range,
                                         const SourceManager &SM,
                                         const LangOptions &LangOpts) {
  SourceLocation Begin = Range.getBegin();
  SourceLocation End = Range.getEnd();
  if (Begin.isInvalid() || End.isInvalid())
    return {};

  if (Begin.isFileID() && End.isFileID())
    return makeRangeFromFileLocs(Range, SM, LangOpts);

  return {};
}

llvm::StringRef Lexer::getSourceText(CharSourceRange Range,
                                     const SourceManager &SM,
                                     const LangOptions &LangOpts,
                                     bool *Invalid) {
  Range = makeFileCharRange(Range, SM, LangOpts);
  if (Range.isInvalid()) {
    if (Invalid)
      *Invalid = true;
    return {};
  }

  // Break down the source location.
  std::pair<FileID, unsigned> beginInfo = SM.getDecomposedLoc(Range.getBegin());
  if (beginInfo.first.isInvalid()) {
    if (Invalid)
      *Invalid = true;
    return {};
  }

  unsigned EndOffs;
  if (!SM.isInFileID(Range.getEnd(), beginInfo.first, &EndOffs) ||
      beginInfo.second > EndOffs) {
    if (Invalid)
      *Invalid = true;
    return {};
  }

  // Try to the load the file buffer.
  bool invalidTemp = false;
  llvm::StringRef file = SM.getBufferData(beginInfo.first, &invalidTemp);
  if (invalidTemp) {
    if (Invalid)
      *Invalid = true;
    return {};
  }

  if (Invalid)
    *Invalid = false;
  return file.substr(beginInfo.second, EndOffs - beginInfo.second);
}

unsigned Lexer::MeasureTokenLength(SourceLocation Loc, const SourceManager &SM,
                                   const LangOptions &LangOpts) {
  Token TheTok;
  if (getRawToken(Loc, TheTok, SM, LangOpts))
    return 0;
  return TheTok.getLength();
}

bool Lexer::getRawToken(SourceLocation Loc, Token &Result,
                        const SourceManager &SM, const LangOptions &LangOpts,
                        bool IgnoreWhiteSpace) {
  // TODO: this could be special cased for common tokens like identifiers, ')',
  // etc to make this faster, if it mattered.  Just look at StrData[0] to handle
  // all obviously single-char tokens.  This could use
  // Lexer::isObviouslySimpleCharacter for example to handle identifiers or
  // something.

  // If this comes from a macro expansion, we really do want the macro name, not
  // the token this macro expanded to.
  Loc = SM.getExpansionLoc(Loc);
  std::pair<FileID, unsigned> LocInfo = SM.getDecomposedLoc(Loc);
  bool Invalid = false;
  llvm::StringRef Buffer = SM.getBufferData(LocInfo.first, &Invalid);
  if (Invalid)
    return true;

  const char *StrData = Buffer.data() + LocInfo.second;

  if (!IgnoreWhiteSpace && isWhitespace(StrData[0]))
    return true;

  // Create a lexer starting at the beginning of this token.
  Lexer TheLexer(SM.getLocForStartOfFile(LocInfo.first), LangOpts,
                 Buffer.begin(), StrData, Buffer.end());
  // TheLexer.SetCommnetRetentionState(true);
  TheLexer.LexFromRawLexer(Result);
  return false;
}

SourceLocation Lexer::getLocForEndOfToken(SourceLocation Loc, unsigned Offset,
                                          const SourceManager &SM,
                                          const LangOptions &LangOpts) {
  if (Loc.isInvalid())
    return {};

  unsigned Len = Lexer::MeasureTokenLength(Loc, SM, LangOpts);
  if (Len > Offset)
    Len = Len - Offset;
  else
    return Loc;

  return Loc.getLocWithOffset(Len);
}

llvm::Optional<Token> Lexer::findNextToken(SourceLocation Loc,
                                           const SourceManager &SM,
                                           const LangOptions &LangOpts) {
  // if (Loc.isMacroID()) {
  //   if (!Lexer::isAtEndOfMacroExpansion(Loc, SM, LangOpts, &Loc))
  //     return None;
  // }
  Loc = Lexer::getLocForEndOfToken(Loc, 0, SM, LangOpts);

  // Break down the source location.
  std::pair<FileID, unsigned> LocInfo = SM.getDecomposedLoc(Loc);

  // Try to load the file buffer.
  bool InvalidTemp = false;
  llvm::StringRef File = SM.getBufferData(LocInfo.first, &InvalidTemp);
  if (InvalidTemp)
    return llvm::None;

  const char *TokenBegin = File.data() + LocInfo.second;

  // Lex from the start of the given location.
  Lexer lexer(SM.getLocForStartOfFile(LocInfo.first), LangOpts, File.begin(),
              TokenBegin, File.end());
  // Find the token.
  Token Tok;
  lexer.LexFromRawLexer(Tok);
  return Tok;
}

static const char *findBeginningOfLine(llvm::StringRef Buffer,
                                       unsigned Offset) {
  const char *BufStart = Buffer.data();
  if (Offset >= Buffer.size())
    return nullptr;

  const char *LexStart = BufStart + Offset;
  for (; LexStart != BufStart; --LexStart) {
    if (isVerticalWhitespace(LexStart[0]) &&
        !Lexer::isNewLineEscaped(BufStart, LexStart)) {
      // LexStart should point at first character of logical line.
      ++LexStart;
      break;
    }
  }
  return LexStart;
}

static SourceLocation getBeginningOfFileToken(SourceLocation Loc,
                                              const SourceManager &SM,
                                              const LangOptions &LangOpts) {
  assert(Loc.isFileID());
  std::pair<FileID, unsigned> LocInfo = SM.getDecomposedLoc(Loc);
  if (LocInfo.first.isValid())
    return Loc;

  bool Invalid = false;
  llvm::StringRef Buffer = SM.getBufferData(LocInfo.first, &Invalid);
  if (Invalid)
    return Loc;

  // Back up from the current location until we hit the beginning of a line
  // (or the buffer). We'll relex from that point.
  const char *StrData = Buffer.data() + LocInfo.second;
  const char *LexStart = findBeginningOfLine(Buffer, LocInfo.second);
  if (!LexStart || LexStart == StrData)
    return Loc;

  // Create a lexer starting at the beginning of this token.
  SourceLocation LexerStartLoc = Loc.getLocWithOffset(-LocInfo.second);
  Lexer TheLexer(LexerStartLoc, LangOpts, Buffer.data(), LexStart,
                 Buffer.end());

  // Lex tokens until we find the token that contains the source location.
  Token TheTok;
  do {
    TheLexer.LexFromRawLexer(TheTok);
    if (TheLexer.getBufferLocation() > StrData) {
      // Lexing this token has taken the lexer past the source location we're
      // looking for. If the current token encompasses our source location,
      // return the beginning of that token.
      if (TheLexer.getBufferLocation() - TheTok.getLength() <= StrData)
        return TheTok.getLocation();

      // We ended up skipping over the source location entirely, which means
      // that it points into whitespace. We're done here.
      break;
    }
  } while (TheTok.getKind() != latino::tok::eof);

  // We've passed our source location; just return the original source location.
  return Loc;
}

SourceLocation Lexer::GetBeginningOfToken(SourceLocation Loc,
                                          const SourceManager &SM,
                                          const LangOptions &LangOpts) {
  if (Loc.isFileID())
    return getBeginningOfFileToken(Loc, SM, LangOpts);
  if (!SM.isMacroArgExpansion(Loc))
    return Loc;
  SourceLocation FileLoc = SM.getSpellingLoc(Loc);
  SourceLocation BeginFileLoc = getBeginningOfFileToken(FileLoc, SM, LangOpts);
  std::pair<FileID, unsigned> FileLocInfo = SM.getDecomposedLoc(FileLoc);
  std::pair<FileID, unsigned> BeginFileLocInfo =
      SM.getDecomposedLoc(BeginFileLoc);
  assert(FileLocInfo.first == BeginFileLocInfo.first &&
         FileLocInfo.second >= BeginFileLocInfo.second);
  return Loc.getLocWithOffset(BeginFileLocInfo.second - FileLocInfo.second);
}

/// getSourceLocation - Return a source location identifier for the specified
/// offset in the current file.
SourceLocation Lexer::getSourceLocation(const char *Loc,
                                        unsigned TokLen) const {
  assert(Loc >= BufferStart && Loc <= BufferEnd &&
         "Location out of range for this buffer!");

  // In the normal case, we're just lexing from a simple file buffer, return
  // the file id from FileLoc with the offset specified.
  unsigned CharNo = Loc - BufferStart;
  if (FileLoc.isFileID())
    return FileLoc.getLocWithOffset(CharNo);

  // Otherwise, this is the _Pragma lexer case, which pretends that all of the
  // tokens are lexed from where the _Pragma was defined.
  // assert(PP && "This doesn't work on raw lexers");
  // return GetMappedTokenLoc(*PP, FileLoc, CharNo, TokLen);
}

/// getEscapedNewLineSize - Return the size of the specified escaped newline,
/// or 0 if it is not an escaped newline. P[-1] is known to be a "\" or a
/// trigraph equivalent on entry to this function.
unsigned Lexer::getEscapedNewLineSize(const char *Ptr) {
  unsigned Size = 0;
  while (isWhitespace(Ptr[Size])) {
    ++Size;

    if (Ptr[Size - 1] != '\n' && Ptr[Size - 1] != '\r')
      continue;

    // If this is a \r\n or \n\r, skip the other half.
    if ((Ptr[Size] == '\r' || Ptr[Size] == '\n') && Ptr[Size - 1] != Ptr[Size])
      ++Size;

    return Size;
  }

  // Not an escaped newline, must be a \t or something else.
  return 0;
}

/// getCharAndSizeSlow - Peek a single 'character' from the specified buffer,
/// get its size, and return it.  This is tricky in several cases:
///   1. If currently at the start of a trigraph, we warn about the trigraph,
///      then either return the trigraph (skipping 3 chars) or the '?',
///      depending on whether trigraphs are enabled or not.
///   2. If this is an escaped newline (potentially with whitespace between
///      the backslash and newline), implicitly skip the newline and return
///      the char after it.
///
/// This handles the slow/uncommon case of the getCharAndSize method.  Here we
/// know that we can accumulate into Size, and that we have already incremented
/// Ptr by Size bytes.
///
/// NOTE: When this method is updated, getCharAndSizeSlowNoWarn (below) should
/// be updated to match.
char Lexer::getCharAndSizeSlow(const char *Ptr, unsigned &Size, Token *Tok) {
  // If we have a slash, look for an escaped newline.
  if (Ptr[0] == '\\') {
    ++Size;
    ++Ptr;
  Slash:
    // Common case, backslash-char where the char is not whitespace.
    if (!isWhitespace(Ptr[0]))
      return '\\';

    // See if we have optional whitespace characters between the slash and
    // newline.
    if (unsigned EscapedNewLineSize = getEscapedNewLineSize(Ptr)) {
      // Remember that this token needs to be cleaned.
      if (Tok)
        Tok->setFlag(Token::NeedsCleaning);

      // Warn if there was whitespace between the backslash and newline.
      // if(Ptr[0] != '\n' && Ptr[0] != '\r' && Tok)
      //   Diag(Ptr, diag::backslash_newline_space);

      // Found backslash<whitespace><newline>.  Parse the char after it.
      Size += EscapedNewLineSize;
      Ptr += EscapedNewLineSize;

      // Use slow version to accumulate a correct size field.
      return getCharAndSizeSlow(Ptr, Size, Tok);
    }

    // Otherwise, this is not an escaped newline, just return the slash.
    return '\\';
  }

  // If this is neither, return a single character.
  ++Size;
  return *Ptr;
}

/// getCharAndSizeSlowNoWarn - Handle the slow/uncommon case of the
/// getCharAndSizeNoWarn method.  Here we know that we can accumulate into Size,
/// and that we have already incremented Ptr by Size bytes.
///
/// NOTE: When this method is updated, getCharAndSizeSlow (above) should
/// be updated to match.
char Lexer::getCharAndSizeSlowNoWarn(const char *Ptr, unsigned &Size,
                                     const LangOptions &LangOpts) {
  // If we have a slash, look for an escaped newline.
  if (Ptr[0] == '\\') {
    ++Size;
    ++Ptr;
  Slash:
    // Common case, backslash-char where the char is not whitespace.
    if (!isWhitespace(Ptr[0]))
      return '\\';

    // See if we have optional whitespace characters followed by a newline.
    if (unsigned EscapedNewLineSize = getEscapedNewLineSize(Ptr)) {
      // Found backslash<whitespace><newline>.  Parse the char after it.
      Size += EscapedNewLineSize;
      Ptr += EscapedNewLineSize;

      // Use slow version to accumulate a correct size field.
      return getCharAndSizeSlowNoWarn(Ptr, Size, LangOpts);
    }

    // Otherwise, this is not an escaped newline, just return the slash.
    return '\\';
  }

  // If this is a trigraph, process it.
  // if (LangOpts.Trigraphs && Ptr[0] == '?' && Ptr[1] == '?') {
  //   // If this is actually a legal trigraph (not something like "??x"),
  //   return
  //   // it.
  //   if (char C = GetTrigraphCharForLetter(Ptr[2])) {
  //     Ptr += 3;
  //     Size += 3;
  //     if (C == '\\')
  //       goto Slash;
  //     return C;
  //   }
  // }

  // If this is neither, return a single character.
  ++Size;
  return *Ptr;
}

/// isHexaLiteral - Return true if Start points to a hex constant.
/// in microsoft mode (where this is supposed to be several different tokens).
bool Lexer::isHexaLiteral(const char *Start, const LangOptions &LangOpts) {
  unsigned Size;
  char C1 = Lexer::getCharAndSizeNoWarn(Start, Size, LangOpts);
  if (C1 != 0)
    return false;
  char C2 = Lexer::getCharAndSizeNoWarn(Start + Size, Size, LangOpts);
  return C2 == 'x' || C2 == 'X';
}

/// LexNumericConstant - Lex the remainder of a integer or floating point
/// constant. From[-1] is the first character lexed.  Return the end of the
/// constant.
bool Lexer::LexNumericConstant(Token &Result, const char *CurPtr) {
  unsigned Size;
  char C = getCharAndSize(CurPtr, Size);
  char PrevCh = 0;
  while (isPreprocessingNumberBody(C)) {
    CurPtr = ConsumeChar(CurPtr, Size, Result);
    PrevCh = C;
    C = getCharAndSize(CurPtr, Size);
  }

  // If we fell out, check for a sign, due to 1e+12.  If we have one, continue.
  if ((C == '-' || C == '+') && (PrevCh == 'E' || PrevCh == 'e')) {
    // If we are in Microsoft mode, don't continue if the constant is hex.
    // For example, MSVC will accept the following as 3 tokens: 0x1234567e+1
    if (!isHexaLiteral(BufferPtr, LangOpts))
      return LexNumericConstant(Result, ConsumeChar(CurPtr, Size, Result));
  }

  // Update the location of token as well as BufferPtr.
  const char *TokStart = BufferPtr;
  FormTokenWithChars(Result, CurPtr, tok::numeric_constant);
  Result.setLiteralData(TokStart);
  return true;
}

/// LexStringLiteral - Lex the remainder of a string literal, after having lexed
/// either " or L" or u8" or u" or U".
bool Lexer::LexStringLiteral(Token &Result, const char *CurPtr,
                             tok::TokenKind Kind) {
  const char *AfterQuote = CurPtr;
  // Does this string contain the \0 character?
  const char *NulCharacter = nullptr;

  char C = getAndAdvanceChar(CurPtr, Result);
  while (C != '"') {
    // Skip escaped characters.  Escaped newlines will already be processed by
    // getAndAdvanceChar.
    if (C == '\\')
      C = getAndAdvanceChar(CurPtr, Result);

    if (C == '\n' || C == '\r'                    /* Newline. */
        || (C == 0 && CurPtr - 1 == BufferEnd)) { /* End of file. */
      FormTokenWithChars(Result, CurPtr - 1, tok::unknown);
      return true;
    }

    if (C == 0) {
      NulCharacter = CurPtr - 1;
    }
    C = getAndAdvanceChar(CurPtr, Result);
  }

  // Update the location of the token as well as the BufferPtr instance var.
  const char *TokStart = BufferPtr;
  FormTokenWithChars(Result, CurPtr, Kind);
  Result.setLiteralData(TokStart);
  return true;
}

/// SkipWhitespace - Efficiently skip over a series of whitespace characters.
/// Update BufferPtr to point to the next non-whitespace character and return.
///
/// This method forms a token and returns true if KeepWhitespaceMode is enabled.
bool Lexer::SkipWhitespace(Token &Result, const char *CurPtr,
                           bool &TokAtPhysicalStartOfLine) {
  // Whitespace - Skip it, then return the token after the whitespace.
  bool SawNewline = isVerticalWhitespace(CurPtr[-1]);

  unsigned char Char = *CurPtr;

  // Skip consecutive spaces efficiently.
  while (true) {
    // Skip horizontal whitespace very aggressively.
    while (isHorizontalWhitespace(Char))
      Char = *++CurPtr;

    // Otherwise if we have something other than whitespace, we're done.
    if (!isVerticalWhitespace(Char))
      break;

    // OK, but handle newline.
    SawNewline = true;
    Char = *++CurPtr;
  }

  // If this isn't immediately after a newline, there is leading space.
  char PrevChar = CurPtr[-1];
  bool HasLeadingSpace = !isVerticalWhitespace(PrevChar);

  Result.setFlagValue(Token::LeadingSpace, HasLeadingSpace);
  if (SawNewline) {
    Result.setFlag(Token::StartOfLine);
    TokAtPhysicalStartOfLine = true;
  }

  BufferPtr = CurPtr;
  return false;
}

/// We have just read the // characters from input.  Skip until we find the
/// newline character that terminates the comment.  Then update BufferPtr and
/// return.
///
/// If we're in KeepCommentMode or any CommentHandler has inserted
/// some tokens, this will store the first token and return true.
bool Lexer::SkipLineComment(Token &Result, const char *CurPtr,
                            bool &TokAtPhysicalStartOfLine) {
  // Scan over the body of the comment.  The common case, when scanning, is that
  // the comment contains normal ascii characters with nothing interesting in
  // them.  As such, optimize for this case with the inner loop.
  //
  // This loop terminates with CurPtr pointing at the newline (or end of buffer)
  // character that ends the line comment.
  char C;
  // Skip over characters in the fast loop.
  while (true) {
    C = *CurPtr;
    if (C == 0                       // Potentially EOF.
        || C == '\n' || C == '\r') { // Newline or DOS-style newline.
      break;
    }
    *(++CurPtr);
  }

  if (C == 0) {
    return LexEndOfFile(Result, CurPtr);
  }
  // Otherwise, eat the \n character.  We don't care if this is a \n\r or
  // \r\n sequence.  This is an efficiency hack (because we know the \n can't
  // contribute to another token), it isn't needed for correctness.  Note that
  // this is ok even in KeepWhitespaceMode, because we would have returned the
  /// comment above in that mode.
  ++CurPtr;

  // The next returned token is at the start of the line.
  Result.setFlag(Token::StartOfLine);
  TokAtPhysicalStartOfLine = true;
  // No leading whitespace seen so far.
  Result.clearFlag(Token::LeadingSpace);
  BufferPtr = CurPtr;
  return false;
}

/// We have just read from input the / and * characters that started a comment.
/// Read until we find the * and / characters that terminate the comment.
/// Note that we don't bother decoding trigraphs or escaped newlines in block
/// comments, because they cannot cause the comment to end.  The only thing
/// that can happen is the comment could end with an escaped newline between
/// the terminating * and /.
///
/// If we're in KeepCommentMode or any CommentHandler has inserted
/// some tokens, this will store the first token and return true.
bool Lexer::SkipBlockComment(Token &Result, const char *CurPtr,
                             bool &TokAtPhysicalStartOfLine) {
  // Scan one character past where we should, looking for a '/' character.  Once
  // we find it, check to see if it was preceded by a *.  This common
  // optimization helps people who like to put a lot of * characters in their
  // comments.

  // The first character we get with newlines and trigraphs skipped to handle
  // the degenerate /*/ case below correctly if the * has an escaped newline
  // after it.
  unsigned CharSize;
  unsigned char C = getCharAndSize(CurPtr, CharSize);
  CurPtr += CharSize;
  if (C == 0 && CurPtr == BufferEnd + 1) {
    // Diag(BufferPtr, diag::err_unterminated_block_comment);
    --CurPtr;
    BufferPtr = CurPtr;
    return false;
  }
  // Check to see if the first character after the '/*' is another /.  If so,
  // then this slash does not end the block comment, it is part of it.
  if (C == '/')
    C = *CurPtr++;

  while (true) {
    // Loop to scan the remainder.
    while (C != '/' && C != '\0')
      C = *CurPtr++;

    if (C == '/') {
      if (CurPtr[-2] == '*')
        break;

      if (CurPtr[0] == '*' && CurPtr[1] != '/') {
        // If this is a /* inside of the comment, emit a warning.  Don't do this
        // if this is a /*/, which will end the comment.  This misses cases with
        // embedded escaped newlines, but oh well.
        // Diag(CurPtr - 1, diag::warn_nested_block_comment);
      }
    } else if (C == 0 && CurPtr == BufferEnd + 1) {
      // Diag(BufferPtr, diag::err_unterminated_block_comment);
      // Note: the user probably forgot a */.  We could continue immediately
      // after the /*, but this would involve lexing a lot of what really is the
      // comment, which surely would confuse the parser.
      --CurPtr;
      BufferPtr = CurPtr;
      return false;
    }

    C = *CurPtr++;
  }
  // It is common for the tokens immediately after a /**/ comment to be
  // whitespace.  Instead of going through the big switch, handle it
  // efficiently now.  This is safe even in KeepWhitespaceMode because we would
  // have already returned above with the comment as a token.
  if (isHorizontalWhitespace(*CurPtr)) {
    SkipWhitespace(Result, CurPtr + 1, TokAtPhysicalStartOfLine);
    return false;
  }

  // Otherwise, just return so that the next character will be lexed as a token.
  BufferPtr = CurPtr;
  Result.setFlag(Token::LeadingSpace);
  return false;
}

bool Lexer::LexIdentifier(Token &Result, const char *CurPtr) {
  // Match [_A-Za-z0-9]*, we have already matched [_A-Za-z$]
  unsigned Size;
  unsigned char C = *CurPtr++;
  while (isIdentifierBody(C))
    C = *CurPtr++;

  --CurPtr; // Back up over the skipped character.

  const char *IdStart = BufferPtr;
  FormTokenWithChars(Result, CurPtr, tok::raw_identifier);
  Result.setRawIdentifierData(IdStart);
  return true;
}

bool Lexer::Lex(Token &Result) {
  // Start a new token.
  Result.startToken();

  // Set up misc whitespace flags for LexTokenInternal.
  if (IsAtStartOfLine) {
    Result.setFlag(Token::StartOfLine);
    IsAtStartOfLine = false;
  }

  bool atPhysicalStartOfLine = IsAtPhysicalStartOfLine;
  IsAtPhysicalStartOfLine = false;
  bool returnedToken = LexTokenInternal(Result, atPhysicalStartOfLine);
  return returnedToken;
}

/// LexEndOfFile - CurPtr points to the end of this file.  Handle this
/// condition, reporting diagnostics and handling other edge cases as required.
/// This returns true if Result contains a token, false if PP.Lex should be
/// called again.
bool Lexer::LexEndOfFile(Token &Result, const char *CurPtr) {
  BufferPtr = CurPtr;
  Result.startToken();
  FormTokenWithChars(Result, BufferEnd, tok::eof);
  return true;
}

/// LexTokenInternal - This implements a simple C family lexer.  It is an
/// extremely performance critical piece of code.  This assumes that the buffer
/// has a null character at the end of the file.  This returns a preprocessing
/// token, not a normal token, as such, it is an internal interface.  It assumes
/// that the Flags of result have been cleared before calling this.
bool Lexer::LexTokenInternal(Token &Result, bool TokAtPhysicalStartOfLine) {
LexNextToken:
  // New token, can't need cleaning yet.
  Result.clearFlag(Token::NeedsCleaning);
  // Result.setIdentifierInfo(nullptr);

  // CurPtr - Cache BufferPtr in an automatic variable.
  const char *CurPtr = BufferPtr;

  // Small amounts of horizontal whitespace is very common between tokens.
  if ((*CurPtr == ' ') || (*CurPtr == '\t')) {
    ++CurPtr;
    while ((*CurPtr == ' ') || (*CurPtr == '\t'))
      ++CurPtr;

    BufferPtr = CurPtr;
    Result.setFlag(Token::LeadingSpace);
  }

  unsigned SizeTmp, SizeTmp2; // Temporaries for use in cases below.

  // Read a character, advancing over it.
  char Char = getAndAdvanceChar(CurPtr, Result);
  tok::TokenKind Kind;

  switch (Char) {
  case 0: // Null.
    // Found end of file?
    if (CurPtr - 1 == BufferEnd)
      return LexEndOfFile(Result, CurPtr - 1);

    // We know the lexer hasn't changed, so just try again with this lexer.
    // (We manually eliminate the tail call to avoid recursion.)
    goto LexNextToken;
  case '\r':
    if (CurPtr[0] == '\n')
      (void)getAndAdvanceChar(CurPtr, Result);
    LLVM_FALLTHROUGH;
  case '\n':
    // No leading whitespace seen so far.
    Result.clearFlag(Token::LeadingSpace);

    if (SkipWhitespace(Result, CurPtr, TokAtPhysicalStartOfLine))
      return true; // KeepWhitespaceMode

    // We only saw whitespace, so just try again with this lexer.
    // (We manually eliminate the tail call to avoid recursion.)
    goto LexNextToken;
  case ' ':
  case '\t':
  case '\f':
  case '\v':
  SkipHorizontalWhitespace:
    Result.setFlag(Token::LeadingSpace);
    if (SkipWhitespace(Result, CurPtr, TokAtPhysicalStartOfLine))
      return true; // KeepWhitespaceMode

  SkipIgnoredUnits:
    CurPtr = BufferPtr;

    // If the next token is obviously a // or /* */ comment, skip it efficiently
    // too (without going through the big switch stmt).
    if (CurPtr[0] == '/' && CurPtr[1] == '/') {
      if (SkipLineComment(Result, CurPtr + 2, TokAtPhysicalStartOfLine))
        return true; // There is a token to return.

      goto SkipIgnoredUnits;
    } else if (isHorizontalWhitespace(*CurPtr)) {
      goto SkipHorizontalWhitespace;
    }
    // We only saw whitespace, so just try again with this lexer.
    // (We manually eliminate the tail call to avoid recursion.)
    goto LexNextToken;

  // C99 6.4.4.1: Integer Constants.
  // C99 6.4.4.2: Floating Constants.
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    // Notify MIOpt that we read a non-whitespace/non-comment token.
    return LexNumericConstant(Result, CurPtr);

    // C99 6.4.2: Identifiers.
  case 'A':
  case 'B':
  case 'C':
  case 'D':
  case 'E':
  case 'F':
  case 'G':
  case 'H':
  case 'I':
  case 'J':
  case 'K': /*'L'*/
  case 'L':
  case 'M':
  case 'N':
  case 'O':
  case 'P':
  case 'Q': /*'R'*/
  case 'R':
  case 'S':
  case 'T': /*'U'*/
  case 'U':
  case 'V':
  case 'W':
  case 'X':
  case 'Y':
  case 'Z':
  case 'a':
  case 'b':
  case 'c':
  case 'd':
  case 'e':
  case 'f':
  case 'g':
  case 'h':
  case 'i':
  case 'j':
  case 'k':
  case 'l':
  case 'm':
  case 'n':
  case 'o':
  case 'p':
  case 'q':
  case 'r':
  case 's':
  case 't': /*'u'*/
  case 'u':
  case 'v':
  case 'w':
  case 'x':
  case 'y':
  case 'z':
  case '_':
    return LexIdentifier(Result, CurPtr);
  case '"':
    return LexStringLiteral(Result, CurPtr, tok::string_literal);
    // C99 6.4.6: Punctuators.
  case '?':
    Kind = tok::question;
    break;
  case '[':
    Kind = tok::l_square;
    break;
  case ']':
    Kind = tok::r_square;
    break;
  case '(':
    Kind = tok::l_paren;
    break;
  case ')':
    Kind = tok::r_paren;
    break;
  case '{':
    Kind = tok::l_brace;
    break;
  case '}':
    Kind = tok::r_brace;
    break;
  case '.':
    Char = getCharAndSize(CurPtr, SizeTmp);
    if (Char >= '0' && Char <= '9') {
      return LexNumericConstant(Result, ConsumeChar(CurPtr, SizeTmp, Result));
    } else if (Char == '.' &&
               getCharAndSize(CurPtr + SizeTmp, SizeTmp2) == '.') {
      Kind = tok::ellipsis;
      CurPtr =
          ConsumeChar(ConsumeChar(CurPtr, SizeTmp, Result), SizeTmp2, Result);
    } else {
      Kind = tok::period;
    }
    break;
  case '&':
    Char = getCharAndSize(CurPtr, SizeTmp);
    if (Char == '&') {
      Kind = tok::ampamp;
      CurPtr = ConsumeChar(CurPtr, SizeTmp, Result);
    } else {
      Kind = tok::unknown;
    }
    break;
  case '*':
    if (getCharAndSize(CurPtr, SizeTmp) == '=') {
      Kind = tok::starequal;
      CurPtr = ConsumeChar(CurPtr, SizeTmp, Result);
    } else {
      Kind = tok::star;
    }
    break;
  case '+':
    Char = getCharAndSize(CurPtr, SizeTmp);
    if (Char == '+') {
      CurPtr = ConsumeChar(CurPtr, SizeTmp, Result);
      Kind = tok::plusplus;
    } else if (Char == '=') {
      CurPtr = ConsumeChar(CurPtr, SizeTmp, Result);
      Kind = tok::plusequal;
    } else {
      Kind = tok::plus;
    }
    break;
  case '-':
    Char = getCharAndSize(CurPtr, SizeTmp);
    if (Char == '-') {
      CurPtr = ConsumeChar(CurPtr, SizeTmp, Result);
      Kind = tok::minusminus;
    } else if (Char == '=') {
      CurPtr = ConsumeChar(CurPtr, SizeTmp, Result);
      Kind = tok::minusequal;
    } else {
      Kind = tok::minus;
    }
    break;
  case '!':
    if (getCharAndSize(CurPtr, SizeTmp) == '=') {
      Kind = tok::exclaimequal;
      CurPtr = ConsumeChar(CurPtr, SizeTmp, Result);
    } else {
      Kind = tok::exclaim;
    }
    break;
  case '#':
    // 6.4.9: Comments python style
    if (SkipLineComment(Result, CurPtr, TokAtPhysicalStartOfLine))
      return true;

    // It is common for the tokens immediately after a // comment to be
    // whitespace (indentation for the next line).  Instead of going through
    // the big switch, handle it efficiently now.
    goto SkipIgnoredUnits;
    break;
  case '/':
    // 6.4.9: Comments
    Char = getCharAndSize(CurPtr, SizeTmp);
    if (Char == '/') {
      // Even if Line comments are disabled (e.g. in C89 mode), we generally
      // want to lex this as a comment.  There is one problem with this though,
      // that in one particular corner case, this can change the behavior of the
      // resultant program.  For example, In  "foo //**/ bar", C89 would lex
      // this as "foo / bar" and languages with Line comments would lex it as
      // "foo".  Check to see if the character after the second slash is a '*'.
      // If so, we will lex that as a "/" instead of the start of a comment.
      // However, we never do this if we are just preprocessing.
      if (SkipLineComment(Result, ConsumeChar(CurPtr, SizeTmp, Result),
                          TokAtPhysicalStartOfLine))
        return true;

      // It is common for the tokens immediately after a // comment to be
      // whitespace (indentation for the next line).  Instead of going through
      // the big switch, handle it efficiently now.
      goto SkipIgnoredUnits;
    }

    if (Char == '*') { // /**/ comment.
      if (SkipBlockComment(Result, ConsumeChar(CurPtr, SizeTmp, Result),
                           TokAtPhysicalStartOfLine))
        return true; // There is a token to return.

      // We only saw whitespace, so just try again with this lexer.
      // (We manually eliminate the tail call to avoid recursion.)
      goto LexNextToken;
    }

    if (Char == '=') {
      CurPtr = ConsumeChar(CurPtr, SizeTmp, Result);
      Kind = tok::slashequal;
    } else {
      Kind = tok::slash;
    }
    break;
  case '%':
    Char = getCharAndSize(CurPtr, SizeTmp);
    if (Char == '=') {
      Kind = tok::percentequal;
      CurPtr = ConsumeChar(CurPtr, SizeTmp, Result);
    } else {
      Kind = tok::percent;
    }
    break;
  case '<':
    Char = getCharAndSize(CurPtr, SizeTmp);
    if (Char == '=') {
      CurPtr = ConsumeChar(CurPtr, SizeTmp, Result);
      Kind = tok::lessequal;
    } else {
      Kind = tok::less;
    }
    break;
  case '>':
    Char = getCharAndSize(CurPtr, SizeTmp);
    if (Char == '=') {
      CurPtr = ConsumeChar(CurPtr, SizeTmp, Result);
      Kind = tok::greaterequal;
    } else {
      Kind = tok::greater;
    }
    break;
  case '^':
    Kind = tok::caret;
    break;
  case '|':
    Char = getCharAndSize(CurPtr, SizeTmp);
    if (Char == '|') {
      CurPtr = ConsumeChar(CurPtr, SizeTmp, Result);
      Kind = tok::pipepipe;
    }
    break;
  case ':':
    Kind = tok::colon;
    break;
  case ';':
    Kind = tok::semi;
    break;
  case '=':
    Char = getCharAndSize(CurPtr, SizeTmp);
    if (Char == '=') {
      Kind = tok::equalequal;
      CurPtr = ConsumeChar(CurPtr, SizeTmp, Result);
    } else {
      Kind = tok::equal;
    }
    break;
  case ',':
    Kind = tok::comma;
    break;
  case '\\':
    Kind = tok::unknown;
    break;
  default: {
    if (isASCII(Char)) {
      Kind = tok::unknown;
      break;
    }
    // We can't just reset CurPtr to BufferPtr because BufferPtr may point to
    // an escaped newline.
    --CurPtr;

    // Non-ASCII characters tend to creep into source code unintentionally.
    // Instead of letting the parser complain about the unknown token,
    // just diagnose the invalid UTF-8, then drop the character.
    // Diag(CurPtr, diag::err_invalid_utf8);

    BufferPtr = CurPtr + 1;
    // We're pretending the character didn't exist, so just try again with
    // this lexer.
    // (We manually eliminate the tail call to avoid recursion.)
    goto LexNextToken;
  }
  }

  // Notify MIOpt that we read a non-whitespace/non-comment token.
  // MIOpt.ReadToken();
  // Update the location of token as well as BufferPtr.
  FormTokenWithChars(Result, CurPtr, Kind);
  return true;
}