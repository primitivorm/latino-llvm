//===- Preprocessor.h - C Language Family Preprocessor ----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Defines the latino::Preprocessor interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_LEX_PREPROCESSOR_H
#define LLVM_LATINO_LEX_PREPROCESSOR_H

#include "latino/Basic/IdentifierTable.h"
#include "latino/Lex/Lexer.h"
#include "latino/Lex/HeaderSearch.h"
#include "latino/Lex/ModuleLoader.h"
#include "latino/Lex/PreprocessorLexer.h"
#include "latino/Lex/Token.h"
#include "latino/Lex/TokenLexer.h"

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/DirectoryLookup.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Basic/Diagnostic.h"

#include "llvm/ADT/SmallVector.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace latino {
class PreprocessorOptions;

/// Engages in a tight little dance with the lexer to efficiently
/// preprocess tokens.
///
/// Lexers know only about tokens within a single source file, and don't
/// know anything about preprocessor-level issues like the \#include stack,
/// token expansion, etc.
class Preprocessor {

  clang::LangOptions       &LangOpts;
  clang::DiagnosticsEngine        *Diags;
  clang::SourceManager     &SourceMgr;
  const clang::TargetInfo *Target = nullptr;
  HeaderSearch      &HeaderInfo;

  std::shared_ptr<PreprocessorOptions> PPOpts;

  /// Cached tokens state.
  using CachedTokensTy = SmallVector<Token, 1>;

  /// Cached tokens are stored here when we do backtracking or
  /// lookahead. They are "lexed" by the CachingLex() method.
  CachedTokensTy CachedTokens;
  /// The position of the cached token that CachingLex() should
  /// "lex" next.
  ///
  /// If it points beyond the CachedTokens vector, it means that a normal
  /// Lex() should be invoked.
  CachedTokensTy::size_type CachedLexPos = 0;

  /// The current top of the stack what we're lexing from
  /// if not expanding a macro.
  ///
  /// This is an alias for CurLexer.
  PreprocessorLexer *CurPPLexer = nullptr;

  /// Used to find the current FileEntry, if CurLexer is non-null
  /// and if applicable.
  ///
  /// This allows us to implement \#include_next and find directory-specific
  /// properties.
  const clang::DirectoryLookup *CurDirLookup = nullptr;

  /// The current macro we are expanding, if we are expanding a macro.
  ///
  /// One of CurLexer and CurTokenLexer must be null.
  std::unique_ptr<TokenLexer> CurTokenLexer;

  /// The kind of lexer we're currently working with.
  enum CurLexerKind {
    CLK_Lexer,
    CLK_TokenLexer,
    CLK_CachingLexer,
    CLK_LexAfterModuleImport
  } CurLexerKind = CLK_Lexer;

  /// If the current lexer is for a submodule that is being built, this
  /// is that submodule.
  Module *CurLexerSubmodule = nullptr;

  /// The number of currently-active calls to Lex.
  ///
  /// Lex is reentrant, and asking for an (end-of-phase-4) token can often
  /// require asking for multiple additional tokens. This counter makes it
  /// possible for Lex to detect whether it's producing a token for the end
  /// of phase 4 of translation or for some other situation.
  unsigned LexLevel = 0;

  /// The number of bytes that we will initially skip when entering the
  /// main file, along with a flag that indicates whether skipping this number
  /// of bytes will place the lexer at the start of a line.
  ///
  /// This is used when loading a precompiled preamble.
  std::pair<int, bool> SkipMainFilePreamble;

  unsigned NumEnteredSourceFiles = 0;
  unsigned MaxIncludeStackDepth = 0;

  /// The offset in file for the code-completion point.
  unsigned CodeCompletionOffset = 0;

  /// The location for the code-completion point. This gets instantiated
  /// when the CodeCompletionFile gets \#include'ed for preprocessing.
  clang::SourceLocation CodeCompletionLoc;

  /// The start location for the file of the code-completion point.
  ///
  /// This gets instantiated when the CodeCompletionFile gets \#include'ed
  /// for preprocessing.
  clang::SourceLocation CodeCompletionFileLoc;

  /// The file that we're performing code-completion for, if any.
  const clang::FileEntry *CodeCompletionFile = nullptr;

  // State that is set before the preprocessor begins.
  bool KeepComments : 1;
  bool KeepMacroComments : 1;

public:
  Preprocessor(std::shared_ptr<clang::PreprocessorOptions> PPOpts,
               clang::DiagnosticsEngine &diags, clang::LangOptions &opts,
               clang::SourceManager &SM, clang::HeaderSearch &Headers,
               ModuleLoader &TheModuleLoader,
               IdentifierInfoLookup *IILookup = nullptr,
               bool OwnsHeaderSearch = false,
               clang::TranslationUnitKind TUKind = clang::TU_Complete);

  ~Preprocessor();

private:

/// The current top of the stack that we're lexing from if
  /// not expanding a macro and we are lexing directly from source code.
  ///
  /// Only one of CurLexer, or CurTokenLexer will be non-null.
  std::unique_ptr<Lexer> CurLexer;

  /// Keeps track of the stack of files currently
  /// \#included, and macros currently being expanded from, not counting
  /// CurLexer/CurTokenLexer.
  struct IncludeStackInfo {
    enum CurLexerKind CurLexerKind;
    Module *TheSubmodule;
    std::unique_ptr<Lexer> TheLexer;
    PreprocessorLexer *ThePPLexer;
    std::unique_ptr<TokenLexer> TheTokenLexer;
    const clang::DirectoryLookup *TheDirLookup;

    // The following constructors are completely useless copies of the default
    // versions, only needed to pacify MSVC.
    IncludeStackInfo(enum CurLexerKind CurLexerKind, Module *TheSubmodule,
                     std::unique_ptr<Lexer> &&TheLexer,
                     PreprocessorLexer *ThePPLexer,
                     std::unique_ptr<TokenLexer> &&TheTokenLexer,
                     const clang::DirectoryLookup *TheDirLookup)
        : CurLexerKind(std::move(CurLexerKind)),
          TheSubmodule(std::move(TheSubmodule)), TheLexer(std::move(TheLexer)),
          ThePPLexer(std::move(ThePPLexer)),
          TheTokenLexer(std::move(TheTokenLexer)),
          TheDirLookup(std::move(TheDirLookup)) {}
  };
  std::vector<IncludeStackInfo> IncludeMacroStack;

  void PushIncludeMacroStack() {
    assert(CurLexerKind != CLK_CachingLexer && "cannot push a caching lexer");
    IncludeMacroStack.emplace_back(CurLexerKind, CurLexerSubmodule,
                                   std::move(CurLexer), CurPPLexer,
                                   std::move(CurTokenLexer), CurDirLookup);
    CurPPLexer = nullptr;
  }

public:

  const clang::LangOptions &getLangOpts() const { return LangOpts; }
  const clang::TargetInfo &getTargetInfo() const { return *Target; }

  clang::SourceManager &getSourceManager() const { return SourceMgr; }

  /// Return the current lexer being lexed from.
  ///
  /// Note that this ignores any potentially active macro expansions and _Pragma
  /// expansions going on at the time.
  PreprocessorLexer *getCurrentLexer() const { return CurPPLexer; }

  /// Lex the next token for this preprocessor.
  void Lex(Token &Result);

  /// Pop the current lexer/macro exp off the top of the lexer stack.
  ///
  /// This should only be used in situations where the current state of the
  /// top-of-stack lexer is known.
  void RemoveTopOfLexerStack();

  //===--------------------------------------------------------------------===//
  // Caching stuff.
  void CachingLex(Token &Result);

  bool InCachingLexMode() const {
    // If the Lexer pointers are 0 and IncludeMacroStack is empty, it means
    // that we are past EOF, not that we are in CachingLex mode.
    return !CurPPLexer && !CurTokenLexer && !IncludeMacroStack.empty();
  }
  void EnterCachingLexMode();
  void EnterCachingLexModeUnchecked();

  void ExitCachingLexMode() {
    if (InCachingLexMode())
      RemoveTopOfLexerStack();
  }

  const Token &PeekAhead(unsigned N);

  /// Peeks ahead N tokens and returns that token without consuming any
  /// tokens.
  ///
  /// LookAhead(0) returns the next token that would be returned by Lex(),
  /// LookAhead(1) returns the token after it, etc.  This returns normal
  /// tokens after phase 5.  As such, it is equivalent to using
  /// 'Lex', not 'LexUnexpandedToken'.
  const Token &LookAhead(unsigned N) {
    assert(LexLevel == 0 && "cannot use lookahead while lexing");
    if (CachedLexPos + N < CachedTokens.size())
      return CachedTokens[CachedLexPos + N];
    else
      return PeekAhead(N + 1);
  }

  /// Enter the specified FileID as the main source file,
  /// which implicitly adds the builtin defines etc.
  void EnterMainSourceFile();

  /// Instruct the preprocessor to skip part of the main source file.
  ///
  /// \param Bytes The number of bytes in the preamble to skip.
  ///
  /// \param StartOfLine Whether skipping these bytes puts the lexer at the
  /// start of a line.
  void setSkipMainFilePreamble(unsigned Bytes, bool StartOfLine) {
    SkipMainFilePreamble.first = Bytes;
    SkipMainFilePreamble.second = StartOfLine;
  }

  /// Computes the source location just past the end of the
  /// token at this source location.
  ///
  /// This routine can be used to produce a source location that
  /// points just past the end of the token referenced by \p Loc, and
  /// is generally used when a diagnostic needs to point just after a
  /// token where it expected something different that it received. If
  /// the returned source location would not be meaningful (e.g., if
  /// it points into a macro), this routine returns an invalid
  /// source location.
  ///
  /// \param Offset an offset from the end of the token, where the source
  /// location should refer to. The default offset (0) produces a source
  /// location pointing just past the end of the token; an offset of 1 produces
  /// a source location pointing to the last character in the token, etc.
  clang::SourceLocation getLocForEndOfToken(clang::SourceLocation Loc,
                                            unsigned Offset = 0) {
    return Lexer::getLocForEndOfToken(Loc, Offset, SourceMgr, LangOpts);
  }

    /// Add a source file to the top of the include stack and
  /// start lexing tokens from it instead of the current buffer.
  ///
  /// Emits a diagnostic, doesn't enter the file, and returns true on error.
  bool EnterSourceFile(clang::FileID FID, const clang::DirectoryLookup *Dir,
                       clang::SourceLocation Loc);

  /// Add a lexer to the top of the include stack and
  /// start lexing tokens from it instead of the current buffer.
  void EnterSourceFileWithLexer(Lexer *TheLexer, const clang::DirectoryLookup *Dir);

  /// Forwarding function for diagnostics.  This emits a diagnostic at
  /// the specified Token's location, translating the token's start
  /// position in the current buffer into a SourcePosition object for rendering.
  clang::DiagnosticBuilder Diag(clang::SourceLocation Loc, unsigned DiagID) const {
    return Diags->Report(Loc, DiagID);
  }

  clang::DiagnosticBuilder Diag(const Token &Tok, unsigned DiagID) const {
    return Diags->Report(Tok.getLocation(), DiagID);
  }

  /// Determine if we are performing code completion.
  bool isCodeCompletionEnabled() const { return CodeCompletionFile != nullptr; }

  bool getCommentRetentionState() const { return KeepComments; }

  /// Control whether the preprocessor retains comments in output.
  void SetCommentRetentionState(bool KeepComments, bool KeepMacroComments) {
    this->KeepComments = KeepComments | KeepMacroComments;
    this->KeepMacroComments = KeepMacroComments;
  }
  
};

} // namespace latino

#endif