#ifndef LATINO_LEX_LEXER_H
#define LATINO_LEX_LEXER_H

#include "Token.h"
#include "latino/Basic/LLVM.h"
#include "latino/Basic/SourceLocation.h"
#include "latino/Basic/SourceManager.h"
//#include "latino/Lex/PreprocessorLexer.h"

namespace latino {
class Lexer {
  const char *BufferStart;
  const char *BufferPtr;
  const char *BufferEnd;
  unsigned char ExtendedTokenMode;
  bool LexingRawMode;
  bool IsAtStartOfLine;
  bool IsAtPhysicalStartOfLine;
  bool HasLeadingSpace;
  bool HasLeadingEmptyMacro;
  SourceLocation FileLoc;

  Lexer(const Lexer &) = delete;
  void operator=(const Lexer &) = delete;
  friend class Preprocessor;
  void InitLexer(const char *BufStart, const char *BufPtr, const char *BufEnd);

private:
  static bool isObviouslySimpleCharacter(char C) { return C != '\\'; }
  char getCharAndSizeSlow(const char *Ptr, unsigned &Size,
                          Token *Tok = nullptr);
  void SetByteOffset(unsigned Offset, bool StartOfLine);
  static unsigned getEscapedNewLineSize(const char *P);
  inline char getAndAdvancedChar(const char *&Ptr, Token &Tok) {
    if (isObviouslySimpleCharacter(Ptr[0]))
      return *Ptr++;
    unsigned Size = 0;
    char C = getCharAndSizeSlow(Ptr, Size, &Tok);
    Ptr += Size;
    return C;
  }
  SourceLocation getSourceLocation(const char *Loc, unsigned TokLen = 1) const;
  SourceLocation getSourceLocation() { return getSourceLocation(BufferPtr); }
  bool LexIdentifier(Token &Result, const char *CurPtr);
  bool LexCharConstant(Token &Result, const char *CurPtr, tok::TokenKind Kind);
  bool LexNumericConstant(Token &Result, const char *CurPtr);
  bool LexStringLiteral(Token &Result, const char *CurPtr, tok::TokenKind Kind);
  bool Lex(Token &Result);
  bool LexTokenInternal(Token &Result, bool TokAtPhysicalStartOfLine);
  bool LexEndOfFile(Token &Result, const char *CurPtr);
  bool LexUnicode(Token &Result, uint32_t C, const char *CurPtr);
  void FormTokenWithChars(Token &Result, const char *TokEnd,
                          tok::TokenKind Kind) {
    unsigned TokLen = TokEnd - BufferPtr;
    Result.setLength(TokLen);
    Result.setLocation(getSourceLocation(BufferPtr, TokLen));
    Result.setKind(Kind);
    BufferPtr = BufferEnd;
  }
  bool SkipWhitespace(Token &Result, const char *CurPtr,
                      bool &TokAtPhysicalStartOfLine);
  bool SkipLineComment(Token &Result, const char *CurPtr,
                       bool &TokAtPhysicalStartOfLine);
  bool SkipBlockComment(Token &Result, const char *CurPtr,
                        bool &TokAtPhysicalStartOfLine);
  inline char getCharAndSize(const char *Ptr, unsigned &Size) {
    if (isObviouslySimpleCharacter(Ptr[0])) {
      Size = 1;
      return *Ptr;
    }
    Size = 0;
    return getCharAndSizeSlow(Ptr, Size);
  }

  const char *ConsumeChar(const char *Ptr, unsigned Size, Token &Tok) {
    if (Size == 1)
      return Ptr + Size;
    Size = 0;
    getCharAndSizeSlow(Ptr, Size, &Tok);
    return Ptr + Size;
  }

  void cutOffLexing() { BufferPtr = BufferEnd; }
  bool isHexaLiteral(const char *Start);
  bool CheckUnicodeWhitespace(Token &Result, uint32_t C, const char *CurPtr);
  void IndirectLex(Token &Result) { Lex(Result); }

public:
  Lexer(FileID FID, const llvm::MemoryBuffer *InputBuffer,
        const SourceManager &SM);

  Lexer(SourceLocation FileLoc, const char *BufStart, const char *BufPtr,
        const char *BufEnd);
  SourceLocation getFileLoc() { return FileLoc; }
  bool isKeepWhitespaceMode() const { return ExtendedTokenMode > 1; }
  void resetExtendedTokenMode();
  static unsigned getSpelling(const Token &Tok, const char *&Buffer,
                              const SourceManager &SourceMgr,
                              bool *Invalid = nullptr);
  std::string getSpelling(const Token &Tok, const SourceManager &SourceMgr,
                          bool *Invalid = nullptr);
  /*static CharSourceRange makeFileCharRange(CharSourceRange Range,
                                           const SourceManager &SM);*/
  /*static SourceLocation getLocForEndOfToken(SourceLocation Loc, unsigned
     Offset, const SourceManager &SM);*/
  /*static unsigned MeasureTokenLength(SourceLocation Loc,
                                     const SourceManager &SM);*/
  /*static bool getRawToken(SourceLocation Loc, Token &Result,
                          const SourceManager &SM,
                          bool IgnoreWhitespace = false);*/
  static SourceLocation GetBeginingOfToken(SourceLocation Loc,
                                           const SourceManager &SM);
  static bool isNewLineEscaped(const char *BufferStart, const char *Str);
  const char *getBufferLocation() const { return BufferPtr; }
  bool LexFromRawLexer(Token &Result) {
    Lex(Result);
    return BufferPtr == BufferEnd;
  }
  void SetCommentRetentionState(bool Mode) {
    assert(!isKeepWhitespaceMode() &&
           "Can't play with retention state when retaining whitespace");
    ExtendedTokenMode = Mode ? 1 : 0;
  }
  static inline char getCharAndSizeNoWarm(const char *Ptr, unsigned &Size) {
    if (isObviouslySimpleCharacter(Ptr[0])) {
      Size = 1;
      return *Ptr;
    }
    Size = 0;
    return getCharAndSizeNoWarm(Ptr, Size);
  }
};

} // namespace latino
#endif /* LATINO_LEX_LEXER_H */
