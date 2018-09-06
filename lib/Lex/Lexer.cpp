#include "latino/Lex/Lexer.h"
#include "UnicodeCharSets.h"
#include "latino/Basic/CharInfo.h"
#include "latino/Basic/IdentifierTable.h"
#include "latino/Basic/LLVM.h"
#include "latino/Lex/Token.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/UnicodeCharRanges.h"

#include <cassert>
#include <iostream>

using namespace std;
using namespace llvm;
using namespace latino;

void Lexer::InitLexer(const char *BuffStart, const char *BuffPtr,
                      const char *BuffEnd) {
  BufferStart = BuffStart;
  BufferPtr = BuffPtr;
  BufferEnd = BuffEnd;
  assert(BuffEnd[0] == 0 &&
         "We assume that the input buffer has a null character at the end"
         " to simplify lexing");
  if (BufferStart == BufferPtr) {
    StringRef Buf(BufferStart, BufferEnd - BufferStart);
    size_t BOMLength =
        StringSwitch<size_t>(Buf).StartsWith("\xEF\xBB\xBF", 3).Default(0);
    BufferPtr += BOMLength;
  }
  IsAtStartOfLine = true;
  IsAtPhysicalStartOfLine = true;
  HasLeadingSpace = false;
}

SourceLocation Lexer::getSourceLocation(const char *Loc,
                                        unsigned TokLen) const {
  assert(Loc >= BufferStart && Loc <= BufferEnd &&
         "Location out of range for this buffer!");
  unsigned CharNo = Loc - BufferStart;
  return FileLoc.getFileLoc(CharNo);
}

Lexer::Lexer(SourceLocation fileloc, const char *BufStart, const char *BufPtr,
             const char *BufEnd)
    : FileLoc(fileloc) {
  InitLexer(BufStart, BufPtr, BufEnd);
  LexingRawMode = true;
}

Lexer::Lexer(FileID FID, const llvm::MemoryBuffer *FromFile,
             const SourceManager &SM)
    : Lexer(SM.getLocForStartOfFile(FID), FromFile->getBufferStart(),
            FromFile->getBufferStart(), FromFile->getBufferEnd()) {}

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

bool Lexer::LexIdentifier(Token &Result, const char *CurPtr) {
  unsigned Size;
  unsigned char C = *CurPtr++;
  while (latino::isIdentifierBody(C))
    C = *CurPtr++;
  --CurPtr;
  if (!latino::isASCII(C)) {
  FinishIdentifier:
    const char *IdStart = BufferPtr;
    FormTokenWithChars(Result, CurPtr, tok::raw_identifier);
    Result.setRawIdentifierData(IdStart);
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

bool Lexer::LexNumericConstant(Token &Result, const char *CurPtr) {
  unsigned Size;
  char C = getCharAndSize(CurPtr, Size);
  char PrevCh = 0;
  while (latino::isPreprocessingNumberBody(C)) {
    CurPtr = ConsumeChar(CurPtr, Size, Result);
    PrevCh = C;
    C = getCharAndSize(CurPtr, Size);
  }
  if ((C == '-' || C == '+') && (PrevCh == 'E' || PrevCh == 'e')) {
    if (!isHexaLiteral(BufferPtr))
      return LexNumericConstant(Result, ConsumeChar(CurPtr, Size, Result));
  }
  const char *TokStart = BufferPtr;
  FormTokenWithChars(Result, CurPtr, tok::numeric_constant);
  Result.setLiteralData(TokStart);
  return true;
}

bool Lexer::isHexaLiteral(const char *Start) {
  unsigned Size;
  char C1 = Lexer::getCharAndSizeNoWarm(Start, Size);
  if (C1 != '0')
    return false;
  char C2 = Lexer::getCharAndSizeNoWarm(Start + Size, Size);
  return (C2 == 'x' || C2 == 'X');
}

bool Lexer::SkipLineComment(Token &Result, const char *CurPtr,
                            bool &TokAtPhysicalStartOfLine) {
  char C;
  while (true) {
    C = *CurPtr;
    while (C != 0 && C != '\n' && C != '\r')
      C = *++CurPtr;
  }
}

bool Lexer::SkipBlockComment(Token &Result, const char *CurPtr,
                             bool &TokAtPhysicalStartOfLine) {
  unsigned CharSize;
  unsigned char C = getCharAndSize(CurPtr, CharSize);
  CurPtr += CharSize;
  if (C == 0 && CurPtr == BufferEnd + 1) {
    --CurPtr;
    BufferPtr = CurPtr;
    return false;
  }
  if (C == '/')
    C = *CurPtr++;
  return false;
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

static size_t getSpellingSlow(const Token &Tok, const char *BufPtr,
                              char *Spelling) {
  assert(Tok.needsCleaning() && "getSpellingSlow called on simple token");
  size_t Length = 0;
  const char *BufEnd = BufPtr + Tok.getLength();
  if (tok::isStringLiteral(Tok.getKind())) {
    while (BufPtr < BufEnd) {
      unsigned Size;
      Spelling[Length++] = Lexer::getCharAndSizeNoWarm(BufPtr, Size);
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
    Spelling[Length++] = Lexer::getCharAndSizeNoWarm(BufPtr, Size);
    BufPtr += Size;
  }
  assert(Length < Tok.getLength() &&
         "NeedsCleaning flag set on token that didn't needs cleaning!");
  return Length;
}

std::string Lexer::getSpelling(const Token &Tok, const SourceManager &SourceMgr,
                               bool *Invalid) {
  assert((int)Tok.getLength() >= 0 && "Token character range is bogus!");
  bool CharDataInvalid = false;
  const char *TokStart =
      SourceMgr.getCharacterData(Tok.getLocation(), &CharDataInvalid);
  if (Invalid)
    *Invalid = CharDataInvalid;
  if (CharDataInvalid)
    return std::string();
  if (!Tok.needsCleaning())
    return std::string(TokStart, TokStart + Tok.getLength());
  std::string Result;
  Result.resize(Tok.getLength());
  Result.resize(getSpellingSlow(Tok, TokStart, &*Result.begin()));
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

static SourceLocation getBeginingOfFileToken(SourceLocation Loc,
                                             const SourceManager &SM) {
  // assert(Loc.isFileID());
  std::pair<FileID, unsigned> LocInfo = SM.getDecomposedLoc(Loc);
  if (LocInfo.first.isInvalid())
    return Loc;
  bool Invalid = false;
  StringRef Buffer = SM.getBufferData(LocInfo.first, &Invalid);
  if (Invalid)
    return Loc;
  const char *StrData = Buffer.data() + LocInfo.second;
  const char *LexStart = findBeginningOfLine(Buffer, LocInfo.second);
  if (!LexStart || LexStart == StrData)
    return Loc;
  SourceLocation LexerStartLoc = Loc.getFileLoc(-LocInfo.second);
  Lexer TheLexer(LexerStartLoc, Buffer.data(), LexStart, Buffer.end());
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

SourceLocation Lexer::GetBeginingOfToken(SourceLocation Loc,
                                         const SourceManager &SM) {
  return getBeginingOfFileToken(Loc, SM);
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
    if (isKeepWhitespaceMode()) {
      FormTokenWithChars(Result, CurPtr, tok::unknown);
      return true;
    }
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
