//===- unittest/AST/CommentTextTest.cpp - Comment text extraction test ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Tests for user-friendly output formatting of comments, i.e.
// RawComment::getFormattedText().
//
//===----------------------------------------------------------------------===//

#include "latino/AST/RawCommentList.h"
#include "latino/Basic/CommentOptions.h"
#include "latino/Basic/Diagnostic.h"
#include "latino/Basic/DiagnosticIDs.h"
#include "latino/Basic/FileManager.h"
#include "latino/Basic/FileSystemOptions.h"
#include "latino/Basic/SourceLocation.h"
#include "latino/Basic/SourceManager.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/VirtualFileSystem.h"
#include <gtest/gtest.h>

namespace latino {

class CommentTextTest : public ::testing::Test {
protected:
  std::string formatComment(llvm::StringRef CommentText) {
    SourceManagerForFile FileSourceMgr("comment-test.cpp", CommentText);
    SourceManager& SourceMgr = FileSourceMgr.get();

    auto CommentStartOffset = CommentText.find("/");
    assert(CommentStartOffset != llvm::StringRef::npos);
    FileID File = SourceMgr.getMainFileID();

    SourceRange CommentRange(
        SourceMgr.getLocForStartOfFile(File).getLocWithOffset(
            CommentStartOffset),
        SourceMgr.getLocForEndOfFile(File));
    CommentOptions EmptyOpts;
    // FIXME: technically, merged that we set here is incorrect, but that
    // shouldn't matter.
    RawComment Comment(SourceMgr, CommentRange, EmptyOpts, /*Merged=*/true);
    DiagnosticsEngine Diags(new DiagnosticIDs, new DiagnosticOptions);
    return Comment.getFormattedText(SourceMgr, Diags);
  }
};

TEST_F(CommentTextTest, FormattedText) {
  // clang-format off
  auto ExpectedOutput =
R"(This function does this and that.
For example,
   Runnning it in that case will give you
   this result.
That's about it.)";
  // Two-slash comments.
  auto Formatted = formatComment(
R"cpp(
// This function does this and that.
// For example,
//    Runnning it in that case will give you
//    this result.
// That's about it.)cpp");
  EXPECT_EQ(ExpectedOutput, Formatted);

  // Three-slash comments.
  Formatted = formatComment(
R"cpp(
/// This function does this and that.
/// For example,
///    Runnning it in that case will give you
///    this result.
/// That's about it.)cpp");
  EXPECT_EQ(ExpectedOutput, Formatted);

  // Block comments.
  Formatted = formatComment(
R"cpp(
/* This function does this and that.
 * For example,
 *    Runnning it in that case will give you
 *    this result.
 * That's about it.*/)cpp");
  EXPECT_EQ(ExpectedOutput, Formatted);

  // Doxygen-style block comments.
  Formatted = formatComment(
R"cpp(
/** This function does this and that.
  * For example,
  *    Runnning it in that case will give you
  *    this result.
  * That's about it.*/)cpp");
  EXPECT_EQ(ExpectedOutput, Formatted);

  // Weird indentation.
  Formatted = formatComment(
R"cpp(
       // This function does this and that.
  //      For example,
  //         Runnning it in that case will give you
        //   this result.
       // That's about it.)cpp");
  EXPECT_EQ(ExpectedOutput, Formatted);
  // clang-format on
}

TEST_F(CommentTextTest, KeepsDoxygenControlSeqs) {
  // clang-format off
  auto ExpectedOutput =
R"(\brief This is the brief part of the comment.
\param a something about a.
@param b something about b.)";

  auto Formatted = formatComment(
R"cpp(
/// \brief This is the brief part of the comment.
/// \param a something about a.
/// @param b something about b.)cpp");
  EXPECT_EQ(ExpectedOutput, Formatted);
  // clang-format on
}

} // namespace latino
