//===--- PPLexerChange.cpp - Handle changing lexers in the preprocessor ---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements pieces of the Preprocessor interface that manage the
// current lexer stack.
//
//===----------------------------------------------------------------------===//

#include "latino/Lex/Preprocessor.h"
#include "latino/Lex/HeaderSearch.h"

#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/DiagnosticLex.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"

using namespace latino;

//===----------------------------------------------------------------------===//
// Methods for Entering and Callbacks for leaving various contexts
//===----------------------------------------------------------------------===//

/// EnterSourceFile - Add a source file to the top of the include stack and
/// start lexing tokens from it instead of the current buffer.
bool Preprocessor::EnterSourceFile(clang::FileID FID,
                                   const clang::DirectoryLookup *CurDir,
                                   clang::SourceLocation Loc) {
  assert(!CurTokenLexer && "Cannot #include a file inside a macro!");
  ++NumEnteredSourceFiles;

  if (MaxIncludeStackDepth < IncludeMacroStack.size())
    MaxIncludeStackDepth = IncludeMacroStack.size();

  // Get the MemoryBuffer for this FID, if it fails, we fail.
  bool Invalid = false;
  const llvm::MemoryBuffer *InputFile =
      getSourceManager().getBuffer(FID, Loc, &Invalid);
  if (Invalid) {
    clang::SourceLocation FileStart = SourceMgr.getLocForStartOfFile(FID);
    Diag(Loc, clang::diag::err_pp_error_opening_file) // TODO: Crear sistema de Diagnostics para latino
        << std::string(SourceMgr.getBufferName(FileStart)) << "";
    return true;
  }

  if (isCodeCompletionEnabled() &&
      SourceMgr.getFileEntryForID(FID) == CodeCompletionFile) {
    CodeCompletionFileLoc = SourceMgr.getLocForStartOfFile(FID);
    CodeCompletionLoc =
        CodeCompletionFileLoc.getLocWithOffset(CodeCompletionOffset);
  }

  EnterSourceFileWithLexer(new Lexer(FID, InputFile, *this), CurDir);
  return false;
}

/// EnterSourceFileWithLexer - Add a source file to the top of the include stack
///  and start lexing tokens from it instead of the current buffer.
void Preprocessor::EnterSourceFileWithLexer(Lexer *TheLexer,
                                            const clang::DirectoryLookup *CurDir) {

  // Add the current lexer to the include stack.
  if (CurPPLexer || CurTokenLexer)
    PushIncludeMacroStack();

  CurLexer.reset(TheLexer);
  CurPPLexer = TheLexer;
  CurDirLookup = CurDir;
  CurLexerSubmodule = nullptr;
  if (CurLexerKind != CLK_LexAfterModuleImport)
    CurLexerKind = CLK_Lexer;

//   // Notify the client, if desired, that we are in a new source file.
//   if (Callbacks && !CurLexer->Is_PragmaLexer) {
//     clang::SrcMgr::CharacteristicKind FileType =
//        SourceMgr.getFileCharacteristic(CurLexer->getFileLoc());

//     Callbacks->FileChanged(CurLexer->getFileLoc(),
//                            PPCallbacks::EnterFile, FileType);
//   }
}
