#include "latino/Lex/Lexer.h"
#include "UnicodeCharSets.h"
#include "latino/Basic/CharInfo.h"
#include "latino/Basic/Diagnostic.h"
#include "latino/Basic/IdentifierTable.h"
#include "latino/Basic/LLVM.h"
#include "latino/Basic/LangOptions.h"
#include "latino/Basic/SourceLocation.h"
#include "latino/Basic/SourceManager.h"
#include "latino/Basic/TokenKinds.h"
#include "latino/Lex/Token.h"
#include "llvm/ADT/None.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/NativeFormatting.h"
#include "llvm/Support/UnicodeCharRanges.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <tuple>
#include <utility>

using namespace latino;

//===----------------------------------------------------------------------===//
// Lexer Class Implementation
//===----------------------------------------------------------------------===//

// void Lexer::anchor() {}

void Lexer::InitLexer(const char *BuffStart, const char *BuffPtr,
                      const char *BuffEnd) {
  BufferStart = BuffStart;
  BufferPtr = BuffPtr;
  BufferEnd = BuffEnd;
  assert(BuffEnd[0] == 0 &&
         "We assume that the input buffer has a null character at the end"
         " to simplify lexing");

  // Check whether we have a BOM in the beginning of the buffer. If yes - act
  // accordingly. Right now we support only UTF-8 with and without BOM, so, just
  // skip the UTF-8 BOM if it's present.
  if (BufferStart == BufferPtr) {
    // Determine the size of the BOM.
    StringRef Buf(BufferStart, BufferEnd - BufferStart);
    size_t BOMLength = llvm::StringSwitch<size_t>(Buf)
                           .StartsWith("\xEF\xBB\xBF", 3) // UTF-8 BOM
                           .Default(0);
    BufferPtr += BOMLength;
  }

  // Start of the file is a start of line.
  IsAtStartOfLine = true;
  IsAtPhysicalStartOfLine = true;

  HasLeadingSpace = false;
}

/// Lexer constructor - Create a new lexer object for the specified buffer
/// with the specified preprocessor managing the lexing process.  This lexer
/// assumes that the associated file buffer and Preprocessor objects will
/// outlive it, so it doesn't take ownership of either of them.
Lexer::Lexer(FileID FID, const llvm::MemoryBuffer *InputFile,
             const SourceManager &SM)
    : FileLoc(SM.getLocForStartOfFile(FID)) {
  InitLexer(InputFile->getBufferStart(), InputFile->getBufferStart(),
            InputFile->getBufferEnd());

  // resetExtendedTokenMode();
}

/// Lexer constructor - Create a new raw lexer object.  This object is only
/// suitable for calls to 'LexFromRawLexer'.  This lexer assumes that the text
/// range will outlive it, so it doesn't take ownership of it.
Lexer::Lexer(SourceLocation fileloc, const LangOptions &langOpts,
             const char *BufStart, const char *BufPtr, const char *BufEnd)
    : FileLoc(fileloc) {
  InitLexer(BufStart, BufPtr, BufEnd);

  // We *are* in raw mode.
  // LexingRawMode = true;
}

/// Lexer constructor - Create a new raw lexer object.  This object is only
/// suitable for calls to 'LexFromRawLexer'.  This lexer assumes that the text
/// range will outlive it, so it doesn't take ownership of it.
Lexer::Lexer(FileID FID, const llvm::MemoryBuffer *FromFile,
             const SourceManager &SM, const LangOptions &langOpts)
    : Lexer(SM.getLocForStartOfFile(FID), langOpts, FromFile->getBufferStart(),
            FromFile->getBufferStart(), FromFile->getBufferEnd()) {}

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
  /*
  // Otherwise, this is the _Pragma lexer case, which pretends that all of the
  // tokens are lexed from where the _Pragma was defined.
  assert(PP && "This doesn't work on raw lexers");
  return GetMappedTokenLoc(*PP, FileLoc, CharNo, TokLen);
  */
}

bool Lexer::Lex(Token &Result) {
  Result.startToken();
  if (IsAtStartOfLine) {
    Result.setFlag(Token::StartOfLine);
    IsAtStartOfLine = false;
  }
  if (HasLeadingSpace) {
    Result.setFlag(Token::LeadingSpace);
    HasLeadingSpace = false;
  }
  if (HasLeadingEmptyMacro) {
    Result.setFlag(Token::LeadingEmptyMacro);
    HasLeadingEmptyMacro = false;
  }
  bool atPhysicalStartOfLine = IsAtPhysicalStartOfLine;
  IsAtPhysicalStartOfLine = false;
  bool returnedToken = LexTokenInternal(Result, atPhysicalStartOfLine);
  return returnedToken;
}

//===----------------------------------------------------------------------===//
// Trigraph and Escaped Newline Handling Code.
//===----------------------------------------------------------------------===//

/*
/// GetTrigraphCharForLetter - Given a character that occurs after a ?? pair,
/// return the decoded trigraph letter it corresponds to, or '\0' if nothing.
static char GetTrigraphCharForLetter(char Letter) {
        switch (Letter) {
        default:   return 0;
        case '=':  return '#';
        case ')':  return ']';
        case '(':  return '[';
        case '!':  return '|';
        case '\'': return '^';
        case '>':  return '}';
        case '/':  return '\\';
        case '<':  return '{';
        case '-':  return '~';
        }
}
*/

bool Lexer::LexIdentifier(Token &Result, const char *CurPtr) {
  // Match [_A-Za-z0-9]*, we have already matched [_A-Za-z$]
  unsigned Size;
  unsigned char C = *CurPtr++;
  while (latino::isIdentifierBody(C))
    C = *CurPtr++;

  --CurPtr; // Back up over the skipped character.

  if (latino::isASCII(C)) {
  FinishIdentifier:
    const char *IdStart = BufferPtr;
    FormTokenWithChars(Result, CurPtr, tok::raw_identifier);
    Result.setRawIdentifierData(IdStart);

    // TODO: Here
    return true;
  }
  return false;
}

bool Lexer::LexCharConstant(Token &Result, const char *CurPtr,
                            tok::TokenKind Kind) {
  const char *NulCharacter = nullptr;
  char C = getAndAdvancedChar(CurPtr, Result);
  if (C == '\'') {
    FormTokenWithChars(Result, CurPtr, tok::unknown);
    return true;
  }
  while (C != '\'') {
    if (C == '\\')
      C = getAndAdvancedChar(CurPtr, Result);
    if (C == '\n' || C == '\r' || (C == 0 && CurPtr - 1 == BufferEnd)) {
      FormTokenWithChars(Result, CurPtr - 1, tok::unknown);
      return true;
    }
    if (C == 0) {
      NulCharacter = CurPtr - 1;
    }
    C = getAndAdvancedChar(CurPtr, Result);
  }
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

    SawNewline = true;
    Char = *++CurPtr;
  }

  // If this isn't immediately after a newline, there is leading space.
  char PrevChar = CurPtr[-1];
  bool HasLeadingSpace = !isVerticalWhitespace(PrevChar);

  // Result.setFlagValue(Token::LeadingSpace, HasLeadingSpace);
  if (SawNewline) {
    Result.setFlag(Token::StartOfLine);
    TokAtPhysicalStartOfLine = true;
  }

  BufferPtr = CurPtr;
  return false;
}

bool Lexer::LexStringLiteral(Token &Result, const char *CurPtr,
                             tok::TokenKind Kind) {
  const char *NulCharacter = nullptr;
  char C = getAndAdvancedChar(CurPtr, Result);
  while (C != '"') {
    if (C == '\\')
      C = getAndAdvancedChar(CurPtr, Result);
    if (C == '\n' || C == '\r' || (C == 0 && CurPtr - 1 == BufferEnd)) {
      FormTokenWithChars(Result, CurPtr - 1, tok::unknown);
      return true;
    }
    if (C == 0) {
      NulCharacter = CurPtr - 1;
    }
    C = getAndAdvancedChar(CurPtr, Result);
  }
  const char *TokStart = BufferPtr;
  FormTokenWithChars(Result, CurPtr, Kind);
  Result.setLiteralData(TokStart);
  return true;
}

/// LexNumericConstant - Lex the remainder of a integer or floating point
/// constant. From[-1] is the first character lexed.  Return the end of the
/// constant.
bool Lexer::LexNumericConstant(Token &Result, const char *CurPtr) {
  unsigned Size;
  char C = getCharAndSize(CurPtr, Size);
  char PrevCh = 0;
  while (latino::isPreprocessingNumberBody(C)) {
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

/// isHexaLiteral - Return true if Start points to a hex constant.
/// in microsoft mode (where this is supposed to be several different tokens).
bool Lexer::isHexaLiteral(const char *Start, const LangOptions &LangOpts) {
  unsigned Size;
  char C1 = Lexer::getCharAndSizeNoWarn(Start, Size, LangOpts);
  if (C1 != '0')
    return false;
  char C2 = Lexer::getCharAndSizeNoWarn(Start + Size, Size, LangOpts);
  return (C2 == 'x' || C2 == 'X');
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
  while (true) {
    C = *CurPtr;
    // Skip over characters in the fast loop.
    while (C != 0 &&               // Potentially EOF.
           C != '\n' && C != '\r') // Newline or DOS-style newline.
      C = *++CurPtr;

    const char *NextLine = CurPtr;
    if (C != 0) {
      // We found a newline, see if it's escaped.
      const char *EscapePtr = CurPtr - 1;
      bool HasSpace = false;
      while (isHorizontalWhitespace(*EscapePtr)) { // Skip whitespace.
        --EscapePtr;
        HasSpace = true;
      }

      if (*EscapePtr == '\\')
        CurPtr = EscapePtr; // Escaped newline.
      else
        break; // This is a newline, we're done.
    }

    // Otherwise, this is a hard case.  Fall back on getAndAdvanceChar to
    // properly decode the character.  Read it in raw mode to avoid emitting
    // diagnostics about things like trigraphs.  If we see an escaped newline,
    // we'll handle it below.
    const char *OldPtr = CurPtr;
    C = getAndAdvancedChar(CurPtr, Result);
    // If we only read only one character, then no special handling is needed.
    // We're done and can skip forward to the newline.
    if (C != 0 && CurPtr == OldPtr + 1) {
      CurPtr = NextLine;
      break;
    }

    // If we read multiple characters, and one of those characters was a \r or
    // \n, then we had an escaped newline within the comment.  Emit diagnostic
    // unless the next line is also a // comment.
    if (CurPtr != OldPtr + 1 && C != '/' &&
        (CurPtr == BufferEnd + 1 || CurPtr[0] != '/')) {
      for (; OldPtr != CurPtr; ++OldPtr)
        if (OldPtr[0] == '\n' || OldPtr[0] == '\r') {
          // Okay, we found a // comment that ends in a newline, if the next
          // line is also a // comment, but has spaces, don't emit a diagnostic.
          if (isWhitespace(C)) {
            const char *ForwardPtr = CurPtr;
            while (isWhitespace(*ForwardPtr))
              ++ForwardPtr; // Skip Whitespace
            if (ForwardPtr[0] == '#' ||
                (ForwardPtr[0] == '/' && ForwardPtr[1] == '/'))
              break;
          }
        }
    }
    if (C == '\r' || C == '\n' || CurPtr == BufferEnd + 1) {
      --CurPtr;
      break;
    }
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

/// isBlockCommentEndOfEscapedNewLine - Return true if the specified newline
/// character (either \\n or \\r) is part of an escaped newline sequence.  Issue
/// a diagnostic if so.  We know that the newline is inside of a block comment.
static bool isEndOfBlockCommentWithEscapedNewLine(const char *CurPtr,
                                                  Lexer *L) {
  assert(CurPtr[0] == '\n' || CurPtr[0] == '\r');

  // Back up off the newline.
  --CurPtr;

  // If this is a two-character newline sequence, skip the other character.
  if (CurPtr[0] == '\n' || CurPtr[0] == '\r') {
    // \n\n or \r\r -> not escaped newline.
    if (CurPtr[0] == CurPtr[1])
      return false;
    // \n\r or \r\n -> skip the newline.
    --CurPtr;
  }

  // If we have horizontal whitespace, skip over it.  We allow whitespace
  // between the slash and newline.
  bool HasSpace = false;
  while (isHorizontalWhitespace(*CurPtr) || *CurPtr == 0) {
    --CurPtr;
    HasSpace = true;
  }
  // If we have a slash, we know this is an escaped newline.
  if (*CurPtr == '\\') {
    if (CurPtr[-1] != '*')
      return false;
  }

  return true;
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
    FoundSlash:
      if (CurPtr[-2] == '*') // We found the final */ We're done!
        break;

      if ((CurPtr[-2] == '\n' || CurPtr[-2] == '\r')) {
        if (isEndOfBlockCommentWithEscapedNewLine(CurPtr - 2, this)) {
          // We found the final */, though it had an escaped newline between the
          // * and /.  We're done!
          break;
        }
      }
    } else if (C == 0 && CurPtr == BufferEnd + 1) {
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

/// LexEndOfFile - CurPtr points to the end of this file.  Handle this
/// condition, reporting diagnostics and handling other edge cases as required.
/// This returns true if Result contains a token, false if PP.Lex should be
/// called again.
bool Lexer::LexEndOfFile(Token &Result, const char *CurPtr) {
  Result.startToken();
  BufferPtr = BufferEnd;
  FormTokenWithChars(Result, BufferEnd, tok::eof);
  return true;
}

unsigned Lexer::getEscapedNewLineSize(const char *Ptr) {
  unsigned Size = 0;
  while (latino::isWhitespace(Ptr[Size])) {
    ++Size;
    if (Ptr[Size - 1] != '\n' && Ptr[Size] != '\r')
      continue;
    if ((Ptr[Size] == '\n' || Ptr[Size] == '\r') && Ptr[Size - 1] != Ptr[Size])
      ++Size;
    return Size;
  }
  return Size;
}

char Lexer::getCharAndSizeSlow(const char *Ptr, unsigned &Size, Token *Tok) {
  if (Ptr[0] == '\\') {
    ++Size;
    ++Ptr;
    if (!latino::isWhitespace(Ptr[0]))
      return '\\';
    if (unsigned EscapedNewLineSize = getEscapedNewLineSize(Ptr)) {
      if (Tok)
        Tok->setFlag(Token::NeedsCleaning);
      Size += EscapedNewLineSize;
      Ptr += EscapedNewLineSize;
      return getCharAndSizeSlow(Ptr, Size, Tok);
    }
    return '\\';
  }
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

    // Slash:
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

  /*
  // If this is a trigraph, process it.
  if (LangOpts.Trigraphs && Ptr[0] == '?' && Ptr[1] == '?') {
          // If this is actually a legal trigraph (not something like "??x"),
  return
          // it.
          if (char C = GetTrigraphCharForLetter(Ptr[2])) {
                  Ptr += 3;
                  Size += 3;
                  if (C == '\\') goto Slash;
                  return C;
          }
  }
  */

  // If this is neither, return a single character.
  ++Size;
  return *Ptr;
}

static size_t getSpellingSlow(const Token &Tok, const char *BufPtr,
                              const LangOptions &LangOpts, char *Spelling) {
  assert(Tok.needsCleaning() && "getSpellingSlow called on simple token");
  size_t Length = 0;
  const char *BufEnd = BufPtr + Tok.getLength();
  if (tok::isStringLiteral(Tok.getKind())) {
    while (BufPtr < BufEnd) {
      unsigned Size;
      Spelling[Length++] = Lexer::getCharAndSizeNoWarn(BufPtr, Size, LangOpts);
      BufPtr += Size;
      if (Spelling[Length - 1] == '"')
        break;
    }
    if (Length >= 2 && Spelling[Length - 1] == '"') {
      const char *RawEnd = BufEnd;
      do
        --RawEnd;
      while (*RawEnd != '"');
      size_t RawLength = RawEnd - BufPtr + 1;
      memcpy(Spelling + Length, BufPtr, RawLength);
      Length += RawLength;
      BufPtr += RawLength;
    }
  }
  while (BufPtr < BufEnd) {
    unsigned Size;
    Spelling[Length++] = Lexer::getCharAndSizeNoWarn(BufPtr, Size, LangOpts);
    BufPtr += Size;
  }
  assert(Length < Tok.getLength() &&
         "NeedsCleaning flag set on token that didn't needs cleaning!");
  return Length;
}

/// getSpelling() - Return the 'spelling' of this token.  The spelling of a
/// token are the characters used to represent the token in the source file
/// after trigraph expansion and escaped-newline folding.  In particular, this
/// wants to get the true, uncanonicalized, spelling of things like digraphs
/// UCNs, etc.
std::string Lexer::getSpelling(const Token &Tok, const SourceManager &SourceMgr,
                               const LangOptions &LangOpts, bool *Invalid) {
  assert((int)Tok.getLength() >= 0 && "Token character range is bogus!");

  bool CharDataInvalid = false;
  const char *TokStart =
      SourceMgr.getCharacterData(Tok.getLocation(), &CharDataInvalid);

  if (Invalid)
    *Invalid = CharDataInvalid;
  if (CharDataInvalid)
    return {};

  // If this token contains nothing interesting, return it directly.
  if (!Tok.needsCleaning())
    return std::string(TokStart, TokStart + Tok.getLength());

  std::string Result;
  Result.resize(Tok.getLength());
  Result.resize(getSpellingSlow(Tok, TokStart, LangOpts, &*Result.begin()));
  return Result;
}

bool Lexer::CheckUnicodeWhitespace(Token &Result, uint32_t C,
                                   const char *CurPtr) {
  static const llvm::sys::UnicodeCharSet UnicodeWhitespaceChars(
      UnicodeWhitespaceCharRanges);
  /*if (!isLexingRawMode() && UnicodeWhitespaceChars.contains(C)) {
    Result.setFlag(Token::LeadingSpace);
    return true;
}*/
  return false;
}

bool Lexer::LexUnicode(Token &Result, uint32_t C, const char *CurPtr) {
  FormTokenWithChars(Result, CurPtr, tok::unknown);
  return true;
}

bool Lexer::isNewLineEscaped(const char *BufferStart, const char *Str) {
  assert(latino::isVerticalWhitespace(Str[0]));
  if (Str - 1 < BufferStart)
    return false;
  if ((Str[0] == '\n' && Str[-1] == '\r') ||
      (Str[0] == '\r' && Str[-1] == '\n')) {
    if (Str - 2 < BufferStart)
      return false;
    --Str;
  }
  --Str;
  while (Str > BufferStart && latino::isHorizontalWhitespace(*Str))
    --Str;
  return *Str == '\\';
}

static const char *findBeginningOfLine(StringRef Buffer, unsigned Offset) {
  const char *BufStart = Buffer.data();
  if (Offset >= Buffer.size())
    return nullptr;
  const char *LexStart = BufStart + Offset;
  for (; LexStart != BufStart; --LexStart) {
    if (latino::isVerticalWhitespace(LexStart[0]) &&
        !Lexer::isNewLineEscaped(BufStart, LexStart)) {
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
  if (LocInfo.first.isInvalid())
    return Loc;

  bool Invalid = false;
  StringRef Buffer = SM.getBufferData(LocInfo.first, &Invalid);
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
  Token TheTok;
  do {
    TheLexer.LexFromRawLexer(TheTok);
    if (TheLexer.getBufferLocation() > StrData) {
      if (TheLexer.getBufferLocation() - TheTok.getLength() <= StrData)
        return TheTok.getLocation();
      break;
    }
  } while (TheTok.getKind() != tok::eof);
  return Loc;
}

SourceLocation Lexer::GetBeginningOfToken(SourceLocation Loc,
                                          const SourceManager &SM,
                                          const LangOptions &LangOpts) {
  return getBeginningOfFileToken(Loc, SM, LangOpts);
}

/*bool Lexer::getRawToken(SourceLocation Loc, Token &Result,
                        const SourceManager &SM, bool IgnoreWhitespace) {
  Loc = SM.getExpansionLoc(Loc);
  std::pair<FileID, unsigned> LocInfo = SM.getDecomposedLoc(Loc);
  bool Invalid = false;
  StringRef Buffer = SM.getBufferData(LocInfo.first, &Invalid);
  if (Invalid)
    return true;
  const char *StrData = Buffer.data() + LocInfo.second;
  if (!IgnoreWhitespace && latino::isWhitespace(StrData[0]))
    return true;
  Lexer TheLexer(SM.getLocForStartOfFile(LocInfo.first), Buffer.begin(),
                 StrData, Buffer.end());
  TheLexer.SetCommentRetentionState(true);
  TheLexer.LexFromRawLexer(Result);
  return false;
}*/

/*unsigned Lexer::MeasureTokenLength(SourceLocation Loc,
                                   const SourceManager &SM) {
  Token TheTok;
  if (getRawToken(Loc, TheTok, SM))
    return 0;
  return TheTok.getLength();
}*/

/*SourceLocation Lexer::getLocForEndOfToken(SourceLocation Loc, unsigned Offset,
                                          const SourceManager &SM) {
  if (Loc.isInvalid())
    return SourceLocation();
  unsigned Len = Lexer::MeasureTokenLength(Loc, SM);
  if (Len > Offset)
    Len = Len - Offset;
  else
    return Loc;
  return Loc.getFileLoc(Len);
}*/

/*static CharSourceRange makeRangeFromFileLocs(CharSourceRange Range,
                                             const SourceManager &SM) {
  SourceLocation Begin = Range.getBegin();
  SourceLocation End = Range.getEnd();
  assert(Begin.isFileID() && End.isFileID());
  if (Range.isTokenRange()) {
    End = Lexer::getLocForEndOfToken(End, 0, SM);
    if (End.isInvalid())
      return CharSourceRange();
  }
  FileID FID;
  unsigned BeginOffs;
  std::tie(FID, BeginOffs) = SM.getDecomposedLoc(Begin);
  if (FID.isInvalid())
    return CharSourceRange();
  unsigned EndOffs;
  if (!SM.isInFileID(End, FID, &EndOffs) || BeginOffs > EndOffs)
    return CharSourceRange();
  return CharSourceRange::getCharRange(Begin, End);
}
*/

/*CharSourceRange Lexer::makeFileCharRange(CharSourceRange Range,
                                         const SourceManager &SM) {
  SourceLocation Begin = Range.getBegin();
  SourceLocation End = Range.getEnd();
  if (Begin.isInvalid() || End.isInvalid())
    return CharSourceRange();
  if (Begin.isFileID() && End.isFileID())
    return makeRangeFromFileLocs(Range, SM);
  bool Invalid = false;
  const latino::SrcMgr::SLocEntry &BeginEntry =
      SM.getSLocEntry(SM.getFileID(Begin), &Invalid);
  if (Invalid)
    return CharSourceRange();
  return CharSourceRange();
}*/

void Lexer::SetByteOffset(unsigned Offset, bool StartOfLine) {
  BufferPtr = BufferStart + Offset;
  if (BufferPtr > BufferEnd)
    BufferPtr = BufferEnd;
  IsAtStartOfLine = StartOfLine;
  IsAtPhysicalStartOfLine = StartOfLine;
}

bool Lexer::LexTokenInternal(Token &Result, bool TokAtPhysicalStartOfLine) {
  unsigned int SizeTmp, SizeTmp2;
LexNextToken:
  Result.clearFlag(Token::NeedsCleaning);
  Result.setIdentifierInfo(nullptr);
  const char *CurPtr = BufferPtr;
  if ((*CurPtr == ' ') || (*CurPtr == '\t')) {
    ++CurPtr;
    while ((*CurPtr == ' ') || (*CurPtr == '\t'))
      ++CurPtr;
    /*if (isKeepWhitespaceMode()) {
      FormTokenWithChars(Result, CurPtr, tok::unknown);
      return true;
    }*/
    BufferPtr = CurPtr;
    Result.setFlag(Token::LeadingSpace);
  }
  char Char = getAndAdvancedChar(CurPtr, Result);
  tok::TokenKind Kind;
  switch (Char) {
  case 0:
    if (CurPtr - 1 == BufferEnd)
      return LexEndOfFile(Result, CurPtr - 1);
    Result.setFlag(Token::LeadingSpace);
    if (SkipWhitespace(Result, CurPtr, TokAtPhysicalStartOfLine))
      return true;
    goto LexNextToken;
  case '\r':
    if (CurPtr[0] == '\n')
      Char = getAndAdvancedChar(CurPtr, Result);
    LLVM_FALLTHROUGH;
  case '\n':
    Result.clearFlag(Token::LeadingSpace);
    if (SkipWhitespace(Result, CurPtr, TokAtPhysicalStartOfLine))
      return true;
    goto LexNextToken;
  case ' ':
  case '\t':
  case '\f':
  case '\v':
  SkipHorizontalWhitespace:
    Result.setFlag(Token::LeadingSpace);
    if (SkipWhitespace(Result, CurPtr, TokAtPhysicalStartOfLine))
      return true;
  SkipIgnoreUnits:
    CurPtr = BufferPtr;
    if (CurPtr[0] == '#') {
      if (SkipLineComment(Result, CurPtr + 1, TokAtPhysicalStartOfLine))
        return true;
    } else if (CurPtr[0] == '/' && CurPtr[1] == '/') {
      if (SkipLineComment(Result, CurPtr + 2, TokAtPhysicalStartOfLine))
        return true;
      goto SkipIgnoreUnits;
    } else if (CurPtr[0] == '/' && CurPtr[1] == '*') {
      if (SkipBlockComment(Result, CurPtr + 2, TokAtPhysicalStartOfLine))
        return true;
      goto SkipIgnoreUnits;
    } else if (latino::isHorizontalWhitespace(*CurPtr)) {
      goto SkipHorizontalWhitespace;
    }
    goto LexNextToken;
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
    return LexNumericConstant(Result, CurPtr);
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
  case 'K':
  case 'L':
  case 'M':
  case 'N':
  case 'O':
  case 'P':
  case 'Q':
  case 'R':
  case 'S':
  case 'T':
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
  case 't':
  case 'u':
  case 'v':
  case 'w':
  case 'x':
  case 'y':
  case 'z':
  case '_':
    return LexIdentifier(Result, CurPtr);
  case '#':
    if (SkipLineComment(Result, CurPtr, TokAtPhysicalStartOfLine))
      return true;
    goto LexNextToken;
  case '\'':
    return LexCharConstant(Result, CurPtr, tok::char_constant);
  case '\"':
    return LexStringLiteral(Result, CurPtr, tok::string_literal);
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
    } else if (Char == '.') {
      Kind = tok::periodperiod;
      CurPtr = ConsumeChar(CurPtr, SizeTmp, Result);
    } else {
      Kind = tok::period;
    }
    break;
  case '&':
    Char = getCharAndSize(CurPtr, SizeTmp);
    if (Char == '&') {
      Kind = tok::ampamp;
      CurPtr = ConsumeChar(CurPtr, SizeTmp, Result);
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
  case '~':
    Char = getCharAndSize(CurPtr, SizeTmp);
    if (Char == '=') {
      CurPtr = ConsumeChar(CurPtr, SizeTmp, Result);
      Kind = tok::tildeequal;
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
  case '/':
    Char = getCharAndSize(CurPtr, SizeTmp);
    if (Char == '/') {
      if (SkipLineComment(Result, ConsumeChar(CurPtr, SizeTmp, Result),
                          TokAtPhysicalStartOfLine))
        return true;
      goto SkipIgnoreUnits;
    }
    if (Char == '*') {
      if (SkipBlockComment(Result, ConsumeChar(CurPtr, SizeTmp, Result),
                           TokAtPhysicalStartOfLine))
        return true;
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
    if (getCharAndSize(CurPtr, SizeTmp) == '=') {
      Kind = tok::lessequal;
      CurPtr = ConsumeChar(CurPtr, SizeTmp, Result);
    } else {
      Kind = tok::less;
    }
    break;
  case '>':
    if (getCharAndSize(CurPtr, SizeTmp) == '=') {
      Kind = tok::greaterequal;
      CurPtr = ConsumeChar(CurPtr, SizeTmp, Result);
    } else {
      Kind = tok::greater;
    }
    break;
  case '|':
    if (getCharAndSize(CurPtr, SizeTmp) == '|') {
      Kind = tok::pipepipe;
      CurPtr = ConsumeChar(CurPtr, SizeTmp, Result);
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
  default:
    if (latino::isASCII(Char)) {
      Kind = tok::unknown;
      break;
    }
    llvm::UTF32 CodePoint;
    --CurPtr;
    llvm::ConversionResult Status = llvm::convertUTF8Sequence(
        (const llvm::UTF8 **)&CurPtr, (const llvm::UTF8 *)BufferEnd, &CodePoint,
        llvm::strictConversion);
    if (Status == llvm::conversionOK) {
      if (CheckUnicodeWhitespace(Result, CodePoint, CurPtr)) {
        if (SkipWhitespace(Result, CurPtr, TokAtPhysicalStartOfLine))
          return true;
        goto LexNextToken;
      }
      return LexUnicode(Result, CodePoint, CurPtr);
    }
    BufferPtr = CurPtr + 1;
    goto LexNextToken;
  } /* end switch */
  FormTokenWithChars(Result, CurPtr, Kind);
  return true;
}
