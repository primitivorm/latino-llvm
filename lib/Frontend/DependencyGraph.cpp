//===--- DependencyGraph.cpp - Generate dependency file -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This code generates a header dependency graph in DOT format, for use
// with, e.g., GraphViz.
//
//===----------------------------------------------------------------------===//

#include "latino/Frontend/Utils.h"
#include "latino/Basic/FileManager.h"
#include "latino/Basic/SourceManager.h"
#include "latino/Frontend/FrontendDiagnostic.h"
#include "latino/Lex/PPCallbacks.h"
#include "latino/Lex/Preprocessor.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/raw_ostream.h"

using namespace latino;
namespace DOT = llvm::DOT;

namespace {
class DependencyGraphCallback : public PPCallbacks {
  const Preprocessor *PP;
  std::string OutputFile;
  std::string SysRoot;
  llvm::SetVector<const FileEntry *> AllFiles;
  typedef llvm::DenseMap<const FileEntry *,
                         SmallVector<const FileEntry *, 2> > DependencyMap;

  DependencyMap Dependencies;

private:
  raw_ostream &writeNodeReference(raw_ostream &OS,
                                  const FileEntry *Node);
  void OutputGraphFile();

public:
  DependencyGraphCallback(const Preprocessor *_PP, StringRef OutputFile,
                          StringRef SysRoot)
    : PP(_PP), OutputFile(OutputFile.str()), SysRoot(SysRoot.str()) { }

  void InclusionDirective(SourceLocation HashLoc, const Token &IncludeTok,
                          StringRef FileName, bool IsAngled,
                          CharSourceRange FilenameRange, const FileEntry *File,
                          StringRef SearchPath, StringRef RelativePath,
                          const Module *Imported,
                          SrcMgr::CharacteristicKind FileType) override;

  void EndOfMainFile() override {
    OutputGraphFile();
  }

};
}

void latino::AttachDependencyGraphGen(Preprocessor &PP, StringRef OutputFile,
                                     StringRef SysRoot) {
  PP.addPPCallbacks(std::make_unique<DependencyGraphCallback>(&PP, OutputFile,
                                                               SysRoot));
}

void DependencyGraphCallback::InclusionDirective(
    SourceLocation HashLoc,
    const Token &IncludeTok,
    StringRef FileName,
    bool IsAngled,
    CharSourceRange FilenameRange,
    const FileEntry *File,
    StringRef SearchPath,
    StringRef RelativePath,
    const Module *Imported,
    SrcMgr::CharacteristicKind FileType) {
  if (!File)
    return;

  SourceManager &SM = PP->getSourceManager();
  const FileEntry *FromFile
    = SM.getFileEntryForID(SM.getFileID(SM.getExpansionLoc(HashLoc)));
  if (!FromFile)
    return;

  Dependencies[FromFile].push_back(File);

  AllFiles.insert(File);
  AllFiles.insert(FromFile);
}

raw_ostream &
DependencyGraphCallback::writeNodeReference(raw_ostream &OS,
                                            const FileEntry *Node) {
  OS << "header_" << Node->getUID();
  return OS;
}

void DependencyGraphCallback::OutputGraphFile() {
  std::error_code EC;
  llvm::raw_fd_ostream OS(OutputFile, EC, llvm::sys::fs::OF_Text);
  if (EC) {
    PP->getDiagnostics().Report(diag::err_fe_error_opening) << OutputFile
                                                            << EC.message();
    return;
  }

  OS << "digraph \"dependencies\" {\n";

  // Write the nodes
  for (unsigned I = 0, N = AllFiles.size(); I != N; ++I) {
    // Write the node itself.
    OS.indent(2);
    writeNodeReference(OS, AllFiles[I]);
    OS << " [ shape=\"box\", label=\"";
    StringRef FileName = AllFiles[I]->getName();
    if (FileName.startswith(SysRoot))
      FileName = FileName.substr(SysRoot.size());

    OS << DOT::EscapeString(std::string(FileName)) << "\"];\n";
  }

  // Write the edges
  for (DependencyMap::iterator F = Dependencies.begin(),
                            FEnd = Dependencies.end();
       F != FEnd; ++F) {
    for (unsigned I = 0, N = F->second.size(); I != N; ++I) {
      OS.indent(2);
      writeNodeReference(OS, F->first);
      OS << " -> ";
      writeNodeReference(OS, F->second[I]);
      OS << ";\n";
    }
  }
  OS << "}\n";
}

