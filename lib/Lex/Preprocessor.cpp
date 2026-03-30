//===- Preprocessor.cpp - C Language Family Preprocessor Implementation ---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements the Preprocessor interface.
//
//===----------------------------------------------------------------------===//
//
// Options to support:
//   -H       - Print the name of each header file used.
//   -d[DNI] - Dump various things.
//   -fworking-directory - #line's with preprocessor's working dir.
//   -fpreprocessed
//   -dependency-file,-M,-MM,-MF,-MG,-MP,-MT,-MQ,-MD,-MMD
//   -W*
//   -w
//
// Messages to emit:
//   "Multiple include guards may be useful for:\n"
//
//===----------------------------------------------------------------------===//

#include "latino/Lex/Preprocessor.h"
#include "latino/Lex/HeaderSearch.h"

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/DiagnosticIDs.h"

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/Capacity.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace latino;

//===----------------------------------------------------------------------===//
// Preprocessor Initialization Methods
//===----------------------------------------------------------------------===//

/// EnterMainSourceFile - Enter the specified FileID as the main source file,
/// which implicitly adds the builtin defines etc.
void Preprocessor::EnterMainSourceFile() {
  // We do not allow the preprocessor to reenter the main file.  Doing so will
  // cause FileID's to accumulate information from both runs (e.g. #line
  // information) and predefined macros aren't guaranteed to be set properly.
  assert(NumEnteredSourceFiles == 0 && "Cannot reenter the main file!");
  clang::FileID MainFileID = SourceMgr.getMainFileID();

  // If MainFileID is loaded it means we loaded an AST file, no need to enter
  // a main file.
  if (!SourceMgr.isLoadedFileID(MainFileID)) {
    // Enter the main file source buffer.
    EnterSourceFile(MainFileID, nullptr, clang::SourceLocation());

     // If we've been asked to skip bytes in the main file (e.g., as part of a
     // precompiled preamble), do so now.
     if (SkipMainFilePreamble.first > 0)
       CurLexer->SetByteOffset(SkipMainFilePreamble.first,
                               SkipMainFilePreamble.second);

    // Tell the header info that the main file was entered.  If the file is later
    // #imported, it won't be re-entered.
    if (const clang::FileEntry *FE = SourceMgr.getFileEntryForID(MainFileID))
      HeaderInfo.IncrementIncludeCount(FE);
  }

  // // Preprocess Predefines to populate the initial preprocessor state.
  // std::unique_ptr<llvm::MemoryBuffer> SB =
  //   llvm::MemoryBuffer::getMemBufferCopy(Predefines, "<built-in>");
  // assert(SB && "Cannot create predefined source buffer");
  // clang::FileID FID = SourceMgr.createFileID(std::move(SB));
  // assert(FID.isValid() && "Could not create FileID for predefines?");
  // setPredefinesFileID(FID);

  // Start parsing the predefines.
  // EnterSourceFile(FID, nullptr, clang::SourceLocation());

  // if (!PPOpts->PCHThroughHeader.empty()) {
  //   // Lookup and save the FileID for the through header. If it isn't found
  //   // in the search path, it's a fatal error.
  //   const clang::DirectoryLookup *CurDir;
  //   Optional<FileEntryRef> File = LookupFile(
  //       clang::SourceLocation(), PPOpts->PCHThroughHeader,
  //       /*isAngled=*/false, /*FromDir=*/nullptr, /*FromFile=*/nullptr, CurDir,
  //       /*SearchPath=*/nullptr, /*RelativePath=*/nullptr,
  //       /*SuggestedModule=*/nullptr, /*IsMapped=*/nullptr,
  //       /*IsFrameworkFound=*/nullptr);
  //   if (!File) {
  //     Diag(clang::SourceLocation(), diag::err_pp_through_header_not_found)
  //         << PPOpts->PCHThroughHeader;
  //     return;
  //   }
  //   setPCHThroughHeaderFileID(
  //       SourceMgr.createFileID(*File, clang::SourceLocation(), SrcMgr::C_User));
  // }

  //// Skip tokens from the Predefines and if needed the main file.
  //if ((usingPCHWithThroughHeader() && SkippingUntilPCHThroughHeader) ||
  //    (usingPCHWithPragmaHdrStop() && SkippingUntilPragmaHdrStop))
  //  SkipTokensWhileUsingPCH();
}
