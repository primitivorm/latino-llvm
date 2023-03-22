//===- unittests/Lex/PPConditionalDirectiveRecordTest.cpp-PP directive tests =//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "latino/Lex/PPConditionalDirectiveRecord.h"
#include "latino/Basic/Diagnostic.h"
#include "latino/Basic/DiagnosticOptions.h"
#include "latino/Basic/FileManager.h"
#include "latino/Basic/LangOptions.h"
#include "latino/Basic/SourceManager.h"
#include "latino/Basic/TargetInfo.h"
#include "latino/Basic/TargetOptions.h"
#include "latino/Lex/HeaderSearch.h"
#include "latino/Lex/HeaderSearchOptions.h"
#include "latino/Lex/ModuleLoader.h"
#include "latino/Lex/Preprocessor.h"
#include "latino/Lex/PreprocessorOptions.h"
#include "gtest/gtest.h"

using namespace latino;

namespace {

// The test fixture.
class PPConditionalDirectiveRecordTest : public ::testing::Test {
protected:
  PPConditionalDirectiveRecordTest()
    : FileMgr(FileMgrOpts),
      DiagID(new DiagnosticIDs()),
      Diags(DiagID, new DiagnosticOptions, new IgnoringDiagConsumer()),
      SourceMgr(Diags, FileMgr),
      TargetOpts(new TargetOptions)
  {
    TargetOpts->Triple = "x86_64-apple-darwin11.1.0";
    Target = TargetInfo::CreateTargetInfo(Diags, TargetOpts);
  }

  FileSystemOptions FileMgrOpts;
  FileManager FileMgr;
  IntrusiveRefCntPtr<DiagnosticIDs> DiagID;
  DiagnosticsEngine Diags;
  SourceManager SourceMgr;
  LangOptions LangOpts;
  std::shared_ptr<TargetOptions> TargetOpts;
  IntrusiveRefCntPtr<TargetInfo> Target;
};

TEST_F(PPConditionalDirectiveRecordTest, PPRecAPI) {
  const char *source =
      "0 1\n"
      "#if 1\n"
      "2\n"
      "#ifndef BB\n"
      "3 4\n"
      "#else\n"
      "#endif\n"
      "5\n"
      "#endif\n"
      "6\n"
      "#if 1\n"
      "7\n"
      "#if 1\n"
      "#endif\n"
      "8\n"
      "#endif\n"
      "9\n";

  std::unique_ptr<llvm::MemoryBuffer> Buf =
      llvm::MemoryBuffer::getMemBuffer(source);
  SourceMgr.setMainFileID(SourceMgr.createFileID(std::move(Buf)));

  TrivialModuleLoader ModLoader;
  HeaderSearch HeaderInfo(std::make_shared<HeaderSearchOptions>(), SourceMgr,
                          Diags, LangOpts, Target.get());
  Preprocessor PP(std::make_shared<PreprocessorOptions>(), Diags, LangOpts,
                  SourceMgr, HeaderInfo, ModLoader,
                  /*IILookup =*/nullptr,
                  /*OwnsHeaderSearch =*/false);
  PP.Initialize(*Target);
  PPConditionalDirectiveRecord *
    PPRec = new PPConditionalDirectiveRecord(SourceMgr);
  PP.addPPCallbacks(std::unique_ptr<PPCallbacks>(PPRec));
  PP.EnterMainSourceFile();

  std::vector<Token> toks;
  while (1) {
    Token tok;
    PP.Lex(tok);
    if (tok.is(tok::eof))
      break;
    toks.push_back(tok);
  }

  // Make sure we got the tokens that we expected.
  ASSERT_EQ(10U, toks.size());
  
  EXPECT_FALSE(PPRec->rangeIntersectsConditionalDirective(
                    SourceRange(toks[0].getLocation(), toks[1].getLocation())));
  EXPECT_TRUE(PPRec->rangeIntersectsConditionalDirective(
                    SourceRange(toks[0].getLocation(), toks[2].getLocation())));
  EXPECT_FALSE(PPRec->rangeIntersectsConditionalDirective(
                    SourceRange(toks[3].getLocation(), toks[4].getLocation())));
  EXPECT_TRUE(PPRec->rangeIntersectsConditionalDirective(
                    SourceRange(toks[1].getLocation(), toks[5].getLocation())));
  EXPECT_TRUE(PPRec->rangeIntersectsConditionalDirective(
                    SourceRange(toks[2].getLocation(), toks[6].getLocation())));
  EXPECT_FALSE(PPRec->rangeIntersectsConditionalDirective(
                    SourceRange(toks[2].getLocation(), toks[5].getLocation())));
  EXPECT_FALSE(PPRec->rangeIntersectsConditionalDirective(
                    SourceRange(toks[0].getLocation(), toks[6].getLocation())));
  EXPECT_TRUE(PPRec->rangeIntersectsConditionalDirective(
                    SourceRange(toks[2].getLocation(), toks[8].getLocation())));
  EXPECT_FALSE(PPRec->rangeIntersectsConditionalDirective(
                    SourceRange(toks[0].getLocation(), toks[9].getLocation())));

  EXPECT_TRUE(PPRec->areInDifferentConditionalDirectiveRegion(
                    toks[0].getLocation(), toks[2].getLocation()));
  EXPECT_FALSE(PPRec->areInDifferentConditionalDirectiveRegion(
                    toks[3].getLocation(), toks[4].getLocation()));
  EXPECT_TRUE(PPRec->areInDifferentConditionalDirectiveRegion(
                    toks[1].getLocation(), toks[5].getLocation()));
  EXPECT_TRUE(PPRec->areInDifferentConditionalDirectiveRegion(
                    toks[2].getLocation(), toks[0].getLocation()));
  EXPECT_FALSE(PPRec->areInDifferentConditionalDirectiveRegion(
                    toks[4].getLocation(), toks[3].getLocation()));
  EXPECT_TRUE(PPRec->areInDifferentConditionalDirectiveRegion(
                    toks[5].getLocation(), toks[1].getLocation()));
}

} // anonymous namespace
