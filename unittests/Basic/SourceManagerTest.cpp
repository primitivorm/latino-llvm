#include "latino/Basic/SourceManager.h"
#include "latino/Basic/Diagnostic.h"
#include "latino/Basic/DiagnosticOptions.h"
#include "latino/Basic/FileManager.h"
#include "latino/Basic/LangOptions.h"
#include "latino/Basic/MemoryBufferCache.h"
#include "latino/Basic/TargetInfo.h"
#include "latino/Basic/TargetOptions.h"
#include "latino/Lex/Lexer.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Config/llvm-config.h"
#include "gtest/gtest.h"

using namespace latino;

namespace {
// The test fixture.
class SourceManagerTest : public ::testing::Test {
protected:
  SourceManagerTest()
      : FileMgr(FileMgrOpts), DiagID(new DiagnosticIDs()),
        Diags(DiagID, new DiagnosticOptions, new IgnoringDiagConsumer()),
        SourceMgr(Diags, FileMgr), TargetOpts(new TargetOptions) {
    TargetOpts->Triple = "i686-pc-windows-msvc";
    // Target = TargetInfo::CreateTargetInfo(Diags, TargetOpts);
  }

  FileSystemOptions FileMgrOpts;
  FileManager FileMgr;
  IntrusiveRefCntPtr<DiagnosticIDs> DiagID;
  DiagnosticsEngine Diags;
  SourceManager SourceMgr;
  LangOptions LangOpts;
  std::shared_ptr<TargetOptions> TargetOpts;
  IntrusiveRefCntPtr<TargetInfo> Target;
}; /* class SourceManagerTest */

TEST_F(SourceManagerTest, isBeforeInTranslationUnit) {
  const char *source = "#este es un comentario \n"
                       "//este es un comentario estilo C \n"
                       "/*este es un comentario\n"
                       "multilinea estilo c*/\n"
                       "imprimir(\"hola mundo\")";
  std::unique_ptr<llvm::MemoryBuffer> Buf =
      llvm::MemoryBuffer::getMemBuffer(source);
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

  // Make sure we got the tokens that we expected.
  ASSERT_EQ(4U, toks.size());
  ASSERT_EQ(tok::raw_identifier, toks[0].getKind());
  ASSERT_EQ(tok::l_paren, toks[1].getKind());
  ASSERT_EQ(tok::string_literal, toks[2].getKind());
  ASSERT_EQ(tok::r_paren, toks[3].getKind());

  SourceLocation idLoc = toks[0].getLocation();
  SourceLocation lparLoc = toks[1].getLocation();
  SourceLocation sLitLoc = toks[2].getLocation();
  SourceLocation rparLoc = toks[3].getLocation();

  EXPECT_TRUE(SourceMgr.isBeforeInTranslationUnit(idLoc, lparLoc));
  EXPECT_TRUE(SourceMgr.isBeforeInTranslationUnit(lparLoc, sLitLoc));
  EXPECT_TRUE(SourceMgr.isBeforeInTranslationUnit(sLitLoc, rparLoc));
}
} // namespace
