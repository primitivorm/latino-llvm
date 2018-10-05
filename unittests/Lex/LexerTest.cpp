#include "latino/Lex/Lexer.h"
#include "latino/Basic/Diagnostic.h"
#include "latino/Basic/DiagnosticIDs.h"
#include "latino/Basic/DiagnosticOptions.h"
#include "latino/Basic/FileManager.h"
#include "latino/Basic/LLVM.h"
#include "latino/Basic/LangOptions.h"
#include "latino/Basic/MemoryBufferCache.h"
#include "latino/Basic/SourceLocation.h"
#include "latino/Basic/SourceManager.h"
#include "latino/Basic/TargetInfo.h"
#include "latino/Basic/TargetOptions.h"
//#include "latino/Frontend/ASTUnit.h"
//#include "latino/Lex/ModuleLoader.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "gtest/gtest.h"

using namespace latino;

namespace {
class LexerTest : public ::testing::Test {
protected:
  LexerTest()
      : FileMgr(FileMgrOpts), DiagID(new DiagnosticIDs()),
        Diags(DiagID, new DiagnosticOptions, new IgnoringDiagConsumer()),
        SourceMgr(Diags, FileMgr), TargetOpts(new TargetOptions) {
    TargetOpts->Triple = "x86_64-unknown-linux-gnu";
    // Target = TargetInfo::CreateTargetInfo(Diags, TargetOpts);
  }

  /*
  std::unique_ptr<Preprocessor> CreatePP(StringRef Source,
                                         TrivialModuleLoader &ModLoader) {
    std::unique_ptr<llvm::MemoryBuffer> Buf =
        llvm::MemoryBuffer::getMemBuffer(Source);
    SourceMgr.setMainFileID(SourceMgr.createFileID(std::move(Buf)));
    MemoryBufferCache PCMCache;
    std::unique_ptr<Preprocessor> PP =
        llvm::make_unique<Preprocessor>(SourceMgr);
    PP->Initialize(*Target);
    PP->EnterMainSourceFile();
    return PP;
  }
  */

  std::vector<Token> Lex(StringRef Source) {
    /*TrivialModuleLoader ModLoader;
    auto PP = CreatePP(Source, ModLoader);*/
    std::unique_ptr<llvm::MemoryBuffer> Buf =
        llvm::MemoryBuffer::getMemBuffer(Source);
    FileID mainFileID = SourceMgr.createFileID(std::move(Buf));
    SourceMgr.setMainFileID(mainFileID);
    SourceLocation Loc;
    bool Invalid = false;
    const llvm::MemoryBuffer *InputFile =
        SourceMgr.getBuffer(mainFileID, Loc, &Invalid);
    std::vector<Token> toks;
    Lexer lexer(mainFileID, InputFile, SourceMgr);
    while (1) {
      Token tok;
      lexer.LexFromRawLexer(tok);
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

  /*std::string getSourceText(Token Begin, Token End) {
    bool Invalid;
    StringRef Str =
        Lexer::getSourceText(CharSourceRange::getTokenRange(SourceRange(
                                 Begin.getLocation(), End.getLocation())),
                             SourceMgr, &Invalid);
    if (Invalid)
      return "<INVALID>";
    return Str;
}*/

  FileSystemOptions FileMgrOpts;
  FileManager FileMgr;
  IntrusiveRefCntPtr<DiagnosticIDs> DiagID;
  DiagnosticsEngine Diags;
  SourceManager SourceMgr;
  LangOptions LangOpts;
  std::shared_ptr<TargetOptions> TargetOpts;
  IntrusiveRefCntPtr<TargetInfo> Target;
};

TEST_F(LexerTest, IsNewLineEscapedValid) {
	auto hasNewLineEscaped = [](const char *S) {
		return Lexer::isNewLineEscaped(S, S + strlen(S) - 1);
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
  const unsigned IdentifierLength = 3;
  std::string TextToLex = "rabarbar\n"
                          "foo\\\nbar\n"
                          "foo\\\rbar\n"
                          "for\\\rnbar\n"
                          "foo\\\n\rba\n";
  std::vector<tok::TokenKind> ExpectedTokens{9, tok::raw_identifier};
  std::vector<Token> LexedTokens = CheckLex(TextToLex, ExpectedTokens);
  for (const Token &Tok : LexedTokens) {
	  std::pair<FileID, unsigned> OriginalLocation =
		  SourceMgr.getDecomposedLoc(Tok.getLocation());
	  for (unsigned Offset = 0; Offset < IdentifierLength; ++Offset) {
		  SourceLocation LookupLocation =
			  Tok.getLocation().getLocWithOffset(Offset);

		  std::pair<FileID, unsigned> FoundLocation =
			  SourceMgr.getDecomposedExpansionLoc(
				  Lexer::GetBeginningOfToken(LookupLocation, SourceMgr, LangOpts));

		  // Check that location returned by the GetBeginningOfToken
		  // is the same as original token location reported by Lexer.
		  EXPECT_EQ(FoundLocation.second, OriginalLocation.second);
	  }
  }
}

TEST_F(LexerTest, AvoidPastEndOfStringDereference) {
	EXPECT_TRUE(Lex(" // \\\n").empty());
	EXPECT_TRUE(Lex("#include <\\\\").empty());
	EXPECT_TRUE(Lex("#include < \\\\\n").empty());
}
} // namespace
