#include "clang/Lex/Lexer.h"
// #include "UnicodeCharSets.h"
#include "clang/Basic/CharInfo.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/LLVM.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TokenKinds.h"
#include "clang/Lex/LexDiagnostic.h"
#include "clang/Lex/LiteralSupport.h"
#include "clang/Lex/MultipleIncludeOpt.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Lex/Token.h"
#include "llvm/ADT/None.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
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
#include <string>
#include <tuple>
#include <utility>

#include "latino/Lex/Lexer.h"

using namespace llvm;
using namespace clang;
using namespace latino;

void latino::Lexer::anchor() {}

void latino::Lexer::InitLexer(const char *BufStart, const char *BufPtr,
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
    // Determine the size of the BOM.
    StringRef Buf(BufferStart, BufferEnd - BufferStart);
    size_t BOMLength = llvm::StringSwitch<size_t>(Buf)
                           .StartsWith("\xEF\xBB\xBF", 3) // UTF-8 BOM
                           .Default(0);

    // Skip the BOM.
    BufferPtr += BOMLength;
  }

  Is_PragmaLexer = false;
  CurrentConflictMarkerState = CMK_None;

  // Start of the file is a start of line.
  IsAtStartOfLine = true;
  IsAtPhysicalStartOfLine = true;

  HasLeadingSpace = false;
  HasLeadingEmptyMacro = false;

  // We are not after parsing a #.
  ParsingPreprocessorDirective = false;

  // We are not after parsing #include.
  ParsingFilename = false;

  // We are not in raw mode.  Raw mode disables diagnostics and interpretation
  // of tokens (e.g. identifiers, thus disabling macro expansion).  It is used
  // to quickly lex the tokens of the buffer, e.g. when handling a "#if 0" block
  // or otherwise skipping over tokens.
  LexingRawMode = false;

  // Default to not keeping comments.
  ExtendedTokenMode = 0;
}

/// Lexer constructor - Create a new lexer object for the specified buffer
/// with the specified preprocessor managing the lexing process.  This lexer
/// assumes that the associated file buffer and Preprocessor objects will
/// outlive it, so it doesn't take ownership of either of them.
latino::Lexer::Lexer(FileID FID, const llvm::MemoryBuffer *InputFile,
                     Preprocessor &PP)
    : PreprocessorLexer(&PP, FID),
      FileLoc(PP.getSourceManager().getLocForStartOfFile(FID)),
      LangOpts(PP.getLangOpts()) {
  InitLexer(InputFile->getBufferStart(), InputFile->getBufferStart(),
            InputFile->getBufferEnd());

  resetExtendedTokenMode();
}

/// Lexer constructor - Create a new raw lexer object.  This object is only
/// suitable for calls to 'LexFromRawLexer'.  This lexer assumes that the text
/// range will outlive it, so it doesn't take ownership of it.
latino::Lexer::Lexer(SourceLocation fileloc, const LangOptions &langOpts,
                     const char *BufStart, const char *BufPtr,
                     const char *BufEnd)
    : FileLoc(fileloc), LangOpts(langOpts) {
  InitLexer(BufStart, BufPtr, BufEnd);

  // We *are* in raw mode.
  LexingRawMode = true;
}

/// Lexer constructor - Create a new raw lexer object.  This object is only
/// suitable for calls to 'LexFromRawLexer'.  This lexer assumes that the text
/// range will outlive it, so it doesn't take ownership of it.
latino::Lexer::Lexer(FileID FID, const llvm::MemoryBuffer *FromFile,
                     const SourceManager &SM, const LangOptions &langOpts)
    : Lexer(SM.getLocForStartOfFile(FID), langOpts, FromFile->getBufferStart(),
            FromFile->getBufferStart(), FromFile->getBufferEnd()) {}

void latino::Lexer::resetExtendedTokenMode() {
  assert(PP && "Cannot reset token mode without a preprocessor");
  if (LangOpts.TraditionalCPP)
    SetKeepWhitespaceMode(true);
  else
    SetCommentRetentionState(PP->getCommentRetentionState());
}

bool latino::Lexer::isNewLineEscaped(const char *BufferStart, const char *Str) {
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

/// Returns the pointer that points to the beginning of line that contains
/// the given offset, or null if the offset if invalid.
static const char *findBeginningOfLine(StringRef Buffer, unsigned Offset) {
  const char *BufStart = Buffer.data();
  if (Offset >= Buffer.size())
    return nullptr;

  const char *LexStart = BufStart + Offset;
  for (; LexStart != BufStart; --LexStart) {
    if (isVerticalWhitespace(LexStart[0]) &&
        !latino::Lexer::isNewLineEscaped(BufStart, LexStart)) {
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
  clang::Lexer TheLexer(LexerStartLoc, LangOpts, Buffer.data(), LexStart,
                        Buffer.end());
  TheLexer.SetCommentRetentionState(true);

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
  } while (TheTok.getKind() != tok::eof);

  // We've passed our source location; just return the original source location.
  return Loc;
}

SourceLocation latino::Lexer::GetBeginningOfToken(SourceLocation Loc,
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

template <typename T> static void StringifyImpl(T &Str, char Quote) {
  typename T::size_type i = 0, e = Str.size();
  while (i < e) {
    if (Str[i] == '\\' || Str[i] == Quote) {
      Str.insert(Str.begin() + i, '\\');
      i += 2;
      ++e;
    } else if (Str[i] == '\n' || Str[i] == '\r') {
      // Replace '\r\n' and '\n\r' to '\\' followed by 'n'.
      if ((i < e - 1) && (Str[i + 1] == '\n' || Str[i + 1] == '\r') &&
          Str[i] != Str[i + 1]) {
        Str[i] = '\\';
        Str[i + 1] = 'n';
      } else {
        // Replace '\n' and '\r' to '\\' followed by 'n'.
        Str[i] = '\\';
        Str.insert(Str.begin() + i + 1, 'n');
        ++e;
      }
      i += 2;
    } else
      ++i;
  }
}

std::string latino::Lexer::Stringify(StringRef Str, bool Charify) {
  std::string Result = std::string(Str);
  char Quote = Charify ? '\'' : '"';
  StringifyImpl(Result, Quote);
  return Result;
}

void latino::Lexer::Stringify(SmallVectorImpl<char> &Str) {
  StringifyImpl(Str, '"');
}

Optional<Token> latino::Lexer::findNextToken(SourceLocation Loc,
                                             const SourceManager &SM,
                                             const LangOptions &LangOpts) {
  // if (Loc.isMacroID()) {
  //   if (!Lexer::isAtEndOfMacroExpansion(Loc, SM, LangOpts, &Loc))
  //     return None;
  // }
  Loc = clang::Lexer::getLocForEndOfToken(Loc, 0, SM, LangOpts);

  // Break down the source location.
  std::pair<FileID, unsigned> LocInfo = SM.getDecomposedLoc(Loc);

  // Try to load the file buffer.
  bool InvalidTemp = false;
  StringRef File = SM.getBufferData(LocInfo.first, &InvalidTemp);
  if (InvalidTemp)
    return None;

  const char *TokenBegin = File.data() + LocInfo.second;

  // Lex from the start of the given location.
  clang::Lexer lexer(SM.getLocForStartOfFile(LocInfo.first), LangOpts,
                     File.begin(), TokenBegin, File.end());
  // Find the token.
  Token Tok;
  lexer.LexFromRawLexer(Tok);
  return Tok;
}

static CharSourceRange makeRangeFromFileLocs(CharSourceRange Range,
                                             const SourceManager &SM,
                                             const LangOptions &LangOpts) {
  SourceLocation Begin = Range.getBegin();
  SourceLocation End = Range.getEnd();
  assert(Begin.isFileID() && End.isFileID());
  if (Range.isTokenRange()) {
    End = clang::Lexer::getLocForEndOfToken(End, 0, SM, LangOpts);
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

CharSourceRange latino::Lexer::makeFileCharRange(CharSourceRange Range,
                                                 const SourceManager &SM,
                                                 const LangOptions &LangOpts) {
  SourceLocation Begin = Range.getBegin();
  SourceLocation End = Range.getEnd();
  if (Begin.isInvalid() || End.isInvalid())
    return {};

  if (Begin.isFileID() && End.isFileID())
    return makeRangeFromFileLocs(Range, SM, LangOpts);

  if (Begin.isMacroID() && End.isFileID()) {
    // if (!isAtStartOfMacroExpansion(Begin, SM, LangOpts, &Begin))
    //   return {};
    Range.setBegin(Begin);
    return makeRangeFromFileLocs(Range, SM, LangOpts);
  }

  if (Begin.isFileID() && End.isMacroID()) {
    // if ((Range.isTokenRange() &&
    //      !isAtEndOfMacroExpansion(End, SM, LangOpts, &End)) ||
    //     (Range.isCharRange() &&
    //      !isAtStartOfMacroExpansion(End, SM, LangOpts, &End)))
    //   return {};
    Range.setEnd(End);
    return makeRangeFromFileLocs(Range, SM, LangOpts);
  }

  assert(Begin.isMacroID() && End.isMacroID());
  SourceLocation MacroBegin, MacroEnd;
  // if (isAtStartOfMacroExpansion(Begin, SM, LangOpts, &MacroBegin) &&
  //     ((Range.isTokenRange() &&
  //       isAtEndOfMacroExpansion(End, SM, LangOpts, &MacroEnd)) ||
  //      (Range.isCharRange() &&
  //       isAtStartOfMacroExpansion(End, SM, LangOpts, &MacroEnd)))) {
  //   Range.setBegin(MacroBegin);
  //   Range.setEnd(MacroEnd);
  //   return makeRangeFromFileLocs(Range, SM, LangOpts);
  // }

  bool Invalid = false;
  const SrcMgr::SLocEntry &BeginEntry =
      SM.getSLocEntry(SM.getFileID(Begin), &Invalid);
  if (Invalid)
    return {};

  if (BeginEntry.getExpansion().isMacroArgExpansion()) {
    const SrcMgr::SLocEntry &EndEntry =
        SM.getSLocEntry(SM.getFileID(End), &Invalid);
    if (Invalid)
      return {};

    if (EndEntry.getExpansion().isMacroArgExpansion() &&
        BeginEntry.getExpansion().getExpansionLocStart() ==
            EndEntry.getExpansion().getExpansionLocStart()) {
      Range.setBegin(SM.getImmediateSpellingLoc(Begin));
      Range.setEnd(SM.getImmediateSpellingLoc(End));
      return makeFileCharRange(Range, SM, LangOpts);
    }
  }

  return {};
}

StringRef latino::Lexer::getSourceText(CharSourceRange Range,
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
  StringRef file = SM.getBufferData(beginInfo.first, &invalidTemp);
  if (invalidTemp) {
    if (Invalid)
      *Invalid = true;
    return {};
  }

  if (Invalid)
    *Invalid = false;
  return file.substr(beginInfo.second, EndOffs - beginInfo.second);
}