#include "clang/Lex/Lexer.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TokenKinds.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/HeaderSearchOptions.h"
#include "clang/Lex/MacroArgs.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/ModuleLoader.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessorOptions.h"

#include "latino/Lex/Lexer.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <vector>

namespace {

using namespace llvm;
using namespace clang;
using namespace latino;

using testing::ElementsAre;

class LexerTest : public ::testing::Test {
protected:
  LexerTest()
      : FileMgr(FileMgrOpts), DiagID(new DiagnosticIDs()),
        Diags(DiagID, new DiagnosticOptions, new IgnoringDiagConsumer()),
        SourceMgr(Diags, FileMgr), TargetOpts(new clang::TargetOptions) {
    TargetOpts->Triple = "x86_64-apple-darwin11.1.0";
    Target = TargetInfo::CreateTargetInfo(Diags, TargetOpts);
  }

  std::unique_ptr<Preprocessor> CreatePP(StringRef Source,
                                         TrivialModuleLoader &ModLoader) {
    std::unique_ptr<llvm::MemoryBuffer> Buf =
        llvm::MemoryBuffer::getMemBuffer(Source);
    SourceMgr.setMainFileID(SourceMgr.createFileID(std::move(Buf)));

    HeaderSearch HeaderInfo(std::make_shared<HeaderSearchOptions>(), SourceMgr,
                            Diags, LangOpts, Target.get());
    std::unique_ptr<Preprocessor> PP = std::make_unique<Preprocessor>(
        std::make_shared<PreprocessorOptions>(), Diags, LangOpts, SourceMgr,
        HeaderInfo, ModLoader,
        /*IILookup =*/nullptr,
        /*OwnsHeaderSearch =*/false);
    PP->Initialize(*Target);
    PP->EnterMainSourceFile();
    return PP;
  }

  std::vector<Token> Lex(StringRef Source) {
    TrivialModuleLoader ModLoader;
    auto PP = CreatePP(Source, ModLoader);

    std::vector<Token> toks;
    while (1) {
      Token tok;
      PP->Lex(tok);
      if (tok.is(tok::eof))
        break;
      toks.push_back(tok);
    }

    return toks;
  }

  std::vector<Token> CheckLex(StringRef Source,
                              ArrayRef<tok::TokenKind> ExpectedTokens) {
    auto toks = Lex(Source);
    EXPECT_EQ(ExpectedTokens.size(), toks.size());
    for (unsigned i = 0, e = ExpectedTokens.size(); i != e; ++i) {
      EXPECT_EQ(ExpectedTokens[i], toks[i].getKind());
    }

    return toks;
  }

    std::string getSourceText(Token Begin, Token End) {
    bool Invalid;
    StringRef Str =
        latino::Lexer::getSourceText(CharSourceRange::getTokenRange(SourceRange(
                                 Begin.getLocation(), End.getLocation())),
                             SourceMgr, LangOpts, &Invalid);
    if (Invalid)
      return "<INVALID>";
    return std::string(Str);
  }

  FileSystemOptions FileMgrOpts;
  FileManager FileMgr;
  IntrusiveRefCntPtr<DiagnosticIDs> DiagID;
  DiagnosticsEngine Diags;
  SourceManager SourceMgr;
  LangOptions LangOpts;
  std::shared_ptr<clang::TargetOptions> TargetOpts;
  IntrusiveRefCntPtr<TargetInfo> Target;
};

TEST_F(LexerTest, DontOverallocateStringifyArgs) {
  TrivialModuleLoader ModLoader;
  auto PP = CreatePP("\"StrArg\", 5, 'C'", ModLoader);

  llvm::BumpPtrAllocator Allocator;
  std::array<IdentifierInfo *, 3> ParamList;
  MacroInfo *MI = PP->AllocateMacroInfo({});
  MI->setIsFunctionLike();
  MI->setParameterList(ParamList, Allocator);
  EXPECT_EQ(3u, MI->getNumParams());
  EXPECT_TRUE(MI->isFunctionLike());

  Token Eof;
  Eof.setKind(tok::eof);
  std::vector<Token> ArgTokens;
  while (1) {
    Token tok;
    PP->Lex(tok);
    if (tok.is(tok::eof)) {
      ArgTokens.push_back(Eof);
      break;
    }
    if (tok.is(tok::comma))
      ArgTokens.push_back(Eof);
    else
      ArgTokens.push_back(tok);
  }

  auto MacroArgsDeleter = [&PP](MacroArgs *M) { M->destroy(*PP); };
  std::unique_ptr<MacroArgs, decltype(MacroArgsDeleter)> MA(
      MacroArgs::create(MI, ArgTokens, false, *PP), MacroArgsDeleter);
  auto StringifyArg = [&](int ArgNo) {
    return MA->StringifyArgument(MA->getUnexpArgument(ArgNo), *PP,
                                 /*Charify=*/false, {}, {});
  };
  Token Result = StringifyArg(0);
  EXPECT_EQ(tok::string_literal, Result.getKind());
  EXPECT_STREQ("\"\\\"StrArg\\\"\"", Result.getLiteralData());
  Result = StringifyArg(1);
  EXPECT_EQ(tok::string_literal, Result.getKind());
  EXPECT_STREQ("\"5\"", Result.getLiteralData());
  Result = StringifyArg(2);
  EXPECT_EQ(tok::string_literal, Result.getKind());
  EXPECT_STREQ("\"'C'\"", Result.getLiteralData());
#if !defined(NDEBUG) && GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(StringifyArg(3), "Invalid arg #");
#endif
}

TEST_F(LexerTest, IsNewLineEscapedValid) {
  auto hasNewLineEscaped = [](const char *S) {
    return latino::Lexer::isNewLineEscaped(S, S + strlen(S) - 1);
  };

  EXPECT_TRUE(hasNewLineEscaped("\\\r"));
  EXPECT_TRUE(hasNewLineEscaped("\\\n"));
  EXPECT_TRUE(hasNewLineEscaped("\\\r\n"));
  EXPECT_TRUE(hasNewLineEscaped("\\\n\r"));
  EXPECT_TRUE(hasNewLineEscaped("\\ \t\v\f\r"));
  EXPECT_TRUE(hasNewLineEscaped("\\ \t\v\f\r\n"));

  EXPECT_FALSE(hasNewLineEscaped("\\\r\r"));
  EXPECT_FALSE(hasNewLineEscaped("\\\r\r\n"));
  EXPECT_FALSE(hasNewLineEscaped("\\\n\n"));
  EXPECT_FALSE(hasNewLineEscaped("\r"));
  EXPECT_FALSE(hasNewLineEscaped("\n"));
  EXPECT_FALSE(hasNewLineEscaped("\r\n"));
  EXPECT_FALSE(hasNewLineEscaped("\n\r"));
  EXPECT_FALSE(hasNewLineEscaped("\r\r"));
  EXPECT_FALSE(hasNewLineEscaped("\n\n"));
}

TEST_F(LexerTest, GetBeginningOfTokenWithEscapedNewLine) {
  // Each line should have the same length for
  // further offset calculation to be more straightforward.
  const unsigned IdentifierLength = 8;
  std::string TextToLex = "rabarbar\n"
                          "foo\\\nbar\n"
                          "foo\\\rbar\n"
                          "fo\\\r\nbar\n"
                          "foo\\\n\rba\n";
  std::vector<tok::TokenKind> ExpectedTokens{5, tok::identifier};
  std::vector<Token> LexedTokens = CheckLex(TextToLex, ExpectedTokens);

  for (const Token &Tok : LexedTokens) {
    std::pair<FileID, unsigned> OriginalLocation =
        SourceMgr.getDecomposedLoc(Tok.getLocation());
    for (unsigned Offset = 0; Offset < IdentifierLength; ++Offset) {
      SourceLocation LookupLocation =
          Tok.getLocation().getLocWithOffset(Offset);

      std::pair<FileID, unsigned> FoundLocation =
          SourceMgr.getDecomposedExpansionLoc(
              latino::Lexer::GetBeginningOfToken(LookupLocation, SourceMgr,
                                                 LangOpts));

      // Check that location returned by the GetBeginningOfToken
      // is the same as original token location reported by Lexer.
      EXPECT_EQ(FoundLocation.second, OriginalLocation.second);
    }
  }
}

TEST_F(LexerTest, StringizingRasString) {
  // For "std::string Lexer::Stringify(StringRef Str, bool Charify)".
  std::string String1 = R"(foo
    {"bar":[]}
    baz)";
  // For "void Lexer::Stringify(SmallVectorImpl<char> &Str)".
  SmallString<128> String2;
  String2 += String1.c_str();

  // Corner cases.
  std::string String3 = R"(\
    \n
    \\n
    \\)";
  SmallString<128> String4;
  String4 += String3.c_str();
  std::string String5 = R"(a\


    \\b)";
  SmallString<128> String6;
  String6 += String5.c_str();

  String1 = latino::Lexer::Stringify(StringRef(String1));
  latino::Lexer::Stringify(String2);
  String3 = latino::Lexer::Stringify(StringRef(String3));
  latino::Lexer::Stringify(String4);
  String5 = latino::Lexer::Stringify(StringRef(String5));
  latino::Lexer::Stringify(String6);

  EXPECT_EQ(String1, R"(foo\n    {\"bar\":[]}\n    baz)");
  EXPECT_EQ(String2, R"(foo\n    {\"bar\":[]}\n    baz)");
  EXPECT_EQ(String3, R"(\\\n    \\n\n    \\\\n\n    \\\\)");
  EXPECT_EQ(String4, R"(\\\n    \\n\n    \\\\n\n    \\\\)");
  EXPECT_EQ(String5, R"(a\\\n\n\n    \\\\b)");
  EXPECT_EQ(String6, R"(a\\\n\n\n    \\\\b)");
}

TEST_F(LexerTest, FindNextToken) {
  Lex("entero abcd = 0;\n"
      "entero xyz = abcd;\n");
  std::vector<std::string> GeneratedByNextToken;
  SourceLocation Loc =
      SourceMgr.getLocForStartOfFile(SourceMgr.getMainFileID());
  while (true) {
    auto T = latino::Lexer::findNextToken(Loc, SourceMgr, LangOpts);
    ASSERT_TRUE(T.hasValue());
    if (T->is(tok::eof))
      break;
    GeneratedByNextToken.push_back(getSourceText(*T, *T));
    Loc = T->getLocation();
  }
  EXPECT_THAT(GeneratedByNextToken, ElementsAre("abcd", "=", "0", ";", "entero",
                                                "xyz", "=", "abcd", ";"));
}

TEST_F(LexerTest, CreatedFIDCountForPredefinedBuffer) {
  TrivialModuleLoader ModLoader;
  auto PP = CreatePP("", ModLoader);
  while (1) {
    Token tok;
    PP->Lex(tok);
    if (tok.is(tok::eof))
      break;
  }
  EXPECT_EQ(SourceMgr.getNumCreatedFIDsForFileID(PP->getPredefinesFileID()),
            1U);
}

} // namespace
