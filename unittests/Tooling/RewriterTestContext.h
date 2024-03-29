//===--- RewriterTestContext.h ----------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines a utility class for Rewriter related tests.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_UNITTESTS_TOOLING_REWRITERTESTCONTEXT_H
#define LLVM_LATINO_UNITTESTS_TOOLING_REWRITERTESTCONTEXT_H

#include "latino/Basic/Diagnostic.h"
#include "latino/Basic/DiagnosticOptions.h"
#include "latino/Basic/FileManager.h"
#include "latino/Basic/LangOptions.h"
#include "latino/Basic/SourceManager.h"
#include "latino/Frontend/TextDiagnosticPrinter.h"
#include "latino/Rewrite/Core/Rewriter.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

namespace latino {

/// \brief A class that sets up a ready to use Rewriter.
///
/// Useful in unit tests that need a Rewriter. Creates all dependencies
/// of a Rewriter with default values for testing and provides convenience
/// methods, which help with writing tests that change files.
class RewriterTestContext {
 public:
   RewriterTestContext()
       : DiagOpts(new DiagnosticOptions()),
         Diagnostics(IntrusiveRefCntPtr<DiagnosticIDs>(new DiagnosticIDs),
                     &*DiagOpts),
         DiagnosticPrinter(llvm::outs(), &*DiagOpts),
         InMemoryFileSystem(new llvm::vfs::InMemoryFileSystem),
         OverlayFileSystem(
             new llvm::vfs::OverlayFileSystem(llvm::vfs::getRealFileSystem())),
         Files(FileSystemOptions(), OverlayFileSystem),
         Sources(Diagnostics, Files), Rewrite(Sources, Options) {
     Diagnostics.setClient(&DiagnosticPrinter, false);
     // FIXME: To make these tests truly in-memory, we need to overlay the
     // builtin headers.
     OverlayFileSystem->pushOverlay(InMemoryFileSystem);
  }

  ~RewriterTestContext() {}

  FileID createInMemoryFile(StringRef Name, StringRef Content) {
    std::unique_ptr<llvm::MemoryBuffer> Source =
        llvm::MemoryBuffer::getMemBuffer(Content);
    InMemoryFileSystem->addFile(Name, 0, std::move(Source));

    auto Entry = Files.getFile(Name);
    assert(Entry);
    return Sources.createFileID(*Entry, SourceLocation(), SrcMgr::C_User);
  }

  // FIXME: this code is mostly a duplicate of
  // unittests/Tooling/RefactoringTest.cpp. Figure out a way to share it.
  FileID createOnDiskFile(StringRef Name, StringRef Content) {
    SmallString<1024> Path;
    int FD;
    std::error_code EC = llvm::sys::fs::createTemporaryFile(Name, "", FD, Path);
    assert(!EC);
    (void)EC;

    llvm::raw_fd_ostream OutStream(FD, true);
    OutStream << Content;
    OutStream.close();
    auto File = Files.getFile(Path);
    assert(File);

    StringRef Found =
        TemporaryFiles.insert(std::make_pair(Name, std::string(Path.str())))
            .first->second;
    assert(Found == Path);
    (void)Found;
    return Sources.createFileID(*File, SourceLocation(), SrcMgr::C_User);
  }

  SourceLocation getLocation(FileID ID, unsigned Line, unsigned Column) {
    SourceLocation Result = Sources.translateFileLineCol(
        Sources.getFileEntryForID(ID), Line, Column);
    assert(Result.isValid());
    return Result;
  }

  std::string getRewrittenText(FileID ID) {
    std::string Result;
    llvm::raw_string_ostream OS(Result);
    Rewrite.getEditBuffer(ID).write(OS);
    OS.flush();
    return Result;
  }

  std::string getFileContentFromDisk(StringRef Name) {
    std::string Path = TemporaryFiles.lookup(Name);
    assert(!Path.empty());
    // We need to read directly from the FileManager without relaying through
    // a FileEntry, as otherwise we'd read through an already opened file
    // descriptor, which might not see the changes made.
    // FIXME: Figure out whether there is a way to get the SourceManger to
    // reopen the file.
    auto FileBuffer = Files.getBufferForFile(Path);
    return std::string((*FileBuffer)->getBuffer());
  }

  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts;
  DiagnosticsEngine Diagnostics;
  TextDiagnosticPrinter DiagnosticPrinter;
  IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> InMemoryFileSystem;
  IntrusiveRefCntPtr<llvm::vfs::OverlayFileSystem> OverlayFileSystem;
  FileManager Files;
  SourceManager Sources;
  LangOptions Options;
  Rewriter Rewrite;

  // Will be set once on disk files are generated.
  llvm::StringMap<std::string> TemporaryFiles;
};

} // end namespace latino

#endif
