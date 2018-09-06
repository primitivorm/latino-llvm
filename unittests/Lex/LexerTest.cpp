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
  FileSystemOptions FileMgrOpts;
  FileManager FileMgr;
  llvm::IntrusiveRefCntPtr<DiagnosticIDs> DiagID;
  DiagnosticsEngine Diags;
  SourceManager SourceMgr;
  std::shared_ptr<TargetOptions> TargetOpts;
  IntrusiveRefCntPtr<TargetInfo> Target;

  LexerTest()
      : FileMgr(FileMgrOpts), DiagID(new DiagnosticIDs()),
        Diags(DiagID, new DiagnosticOptions, new IgnoringDiagConsumer()),
        SourceMgr(Diags, FileMgr), TargetOpts(new TargetOptions) {
    TargetOpts->Triple = "x86_64-unknown-linux-gnu";
    Target = TargetInfo::CreateTargetInfo(Diags, TargetOpts);
  }

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
};

TEST_F(LexerTest, GetBeginingOfTokenWithEscapedNewLine) {
  const unsigned IdentifierLength = 8;
  std::string TextToLex = "rabarbar\n"
                          "foo\\\nbar\n"
                          "foo\\\rbar\n"
                          "fo\\\rnbar\n"
                          "foo\\\n\rba\n";
  std::vector<tok::TokenKind> ExpectedTokens{5, tok::identifier};
  std::vector<Token> LexedTokens = CheckLex(TextToLex, ExpectedTokens);
  for (const Token &Tok : LexedTokens) {
    std::pair<FileID, unsigned> OriginalLocation =
        SourceMgr.getDecomposedLoc(Tok.getLocation());
    for (unsigned Offset = 0; Offset < IdentifierLength; ++Offset) {
      SourceLocation LookupLocation = Tok.getLocation().getLocWithOffset(Offset);
      std::pair<FileID, unsigned> FoundLocation =
          SourceMgr.getDecomposedExpansionLoc(
              Lexer::GetBeginingOfToken(LookupLocation, SourceMgr));
      EXPECT_EQ(FoundLocation.second, OriginalLocation.second);
    }
  }
}
} // namespace
