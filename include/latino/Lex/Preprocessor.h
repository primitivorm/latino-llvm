#ifndef LLVM_LATINO_LEX_PREPROCESSOR_H
#define LLVM_LATINO_LEX_PREPROCESSOR_H

#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Lex/TokenLexer.h"

#include "latino/Basic/Diagnostic.h"
#include "latino/Basic/IdentifierTable.h"
#include "latino/Basic/LangOptions.h"
#include "latino/Basic/Module.h"
#include "latino/Basic/SourceManager.h"
#include "latino/Lex/DirectoryLookup.h"
#include "latino/Lex/HeaderSearch.h"
#include "latino/Lex/Lexer.h"
#include "latino/Lex/ModuleLoader.h"
#include "latino/Lex/Token.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/FoldingSet.h"
#include "llvm/ADT/FunctionExtras.h"
#include "llvm/ADT/None.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/PointerUnion.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/TinyPtrVector.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Registry.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace clang {
class clang::PreprocessorOptions;
// class HeaderSearch;

class clang::TokenLexer;

} // namespace clang

namespace latino {

    namespace Builtin {
class Context;
}

class TargetInfo;
class DirectoryLookup;
class PreprocessorLexer;

class Preprocessor {

  llvm::unique_function<void(const Token &)> OnToken;

  std::shared_ptr<clang::PreprocessorOptions> PPOpts;
  DiagnosticsEngine *Diags;
  LangOptions &LangOpts;
  const TargetInfo *Target = nullptr;
  const TargetInfo *AuxTarget = nullptr;
  FileManager &FileMgr;
  SourceManager &SourceMgr;
  // clang::HeaderSearch &HeaderInfo;
  ModuleLoader &TheModuleLoader;

  /// Mapping/lookup information for all identifiers in
  /// the program, including program keywords.
  mutable IdentifierTable Identifiers;

  /// True if we want to ignore EOF token and continue later on (thus
  /// avoid tearing the Lexer and etc. down).
  bool IncrementalProcessing = false;

  /// The kind of translation unit we are processing.
  TranslationUnitKind TUKind;

  /// The number of currently-active calls to Lex.
  ///
  /// Lex is reentrant, and asking for an (end-of-phase-4) token can often
  /// require asking for multiple additional tokens. This counter makes it
  /// possible for Lex to detect whether it's producing a token for the end
  /// of phase 4 of translation or for some other situation.
  unsigned LexLevel = 0;

  /// The number of (LexLevel 0) preprocessor tokens.
  unsigned TokenCount = 0;

  /// The maximum number of (LexLevel 0) tokens before issuing a -Wmax-tokens
  /// warning, or zero for unlimited.
  unsigned MaxTokens = 0;
  SourceLocation MaxTokensOverrideLoc;

  /// Whether the last token we lexed was an '@'.
  bool LastTokenWasAt = false;

  /// Information about builtins.
  std::unique_ptr<Builtin::Context> BuiltinInfo;

  // Various statistics we track for performance analysis.
  unsigned NumEnteredSourceFiles = 0;

private:
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
  const DirectoryLookup *CurDirLookup = nullptr;

  /// The current macro we are expanding, if we are expanding a macro.
  ///
  /// One of CurLexer and CurTokenLexer must be null.
  std::unique_ptr<clang::TokenLexer> CurTokenLexer;

  /// Cached tokens state.
  using CachedTokensTy = llvm::SmallVector<Token, 1>;

  /// Cached tokens are stored here when we do backtracking or
  /// lookahead. They are "lexed" by the CachingLex() method.
  CachedTokensTy CachedTokens;

  /// The position of the cached token that CachingLex() should
  /// "lex" next.
  ///
  /// If it points beyond the CachedTokens vector, it means that a normal
  /// Lex() should be invoked.
  CachedTokensTy::size_type CachedLexPos = 0;

  /// The current top of the stack that we're lexing from if
  /// not expanding a macro and we are lexing directly from source code.
  ///
  /// Only one of CurLexer, or CurTokenLexer will be non-null.
  std::unique_ptr<Lexer> CurLexer;

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

  /// \{
  /// Cache of macro expanders to reduce malloc traffic.
  enum { TokenLexerCacheSize = 8 };
  unsigned NumCachedTokenLexers;
  std::unique_ptr<clang::TokenLexer> TokenLexerCache[TokenLexerCacheSize];
  /// \}

  void PopIncludeMacroStack() {}

public:
  Preprocessor(std::shared_ptr<clang::PreprocessorOptions> PPOpts,
               DiagnosticsEngine &diags, LangOptions &opts, SourceManager &SM,
               HeaderSearch &Headers, ModuleLoader &TheModuleLoader,
               IdentifierInfoLookup *IILookup = nullptr,
               bool OwnsHeaderSearch = false,
               TranslationUnitKind TUKind = TranslationUnitKind::TU_Complete);

  ~Preprocessor();

  /// Initialize the preprocessor using information about the target.
  ///
  /// \param Target is owned by the caller and must remain valid for the
  /// lifetime of the preprocessor.
  /// \param AuxTarget is owned by the caller and must remain valid for
  /// the lifetime of the preprocessor.
  void Initialize(const TargetInfo &Target,
                  const TargetInfo *AuxTarget = nullptr);

  /// Enter the specified FileID as the main source file,
  /// which implicitly adds the builtin defines etc.
  void EnterMainSourceFile();

  /// Inform the preprocessor callbacks that processing is complete.
  // void EndSourceFile();

  /// Add a source file to the top of the include stack and
  /// start lexing tokens from it instead of the current buffer.
  ///
  /// Emits a diagnostic, doesn't enter the file, and returns true on error.
  bool EnterSourceFile(FileID FID, const DirectoryLookup *Dir,
                       SourceLocation Loc);

  DiagnosticsEngine &getDiagnostics() const { return *Diags; }
  void setDiagnostics(DiagnosticsEngine &D) { Diags = &D; }

  const LangOptions &getLangOpts() const { return LangOpts; }
  const TargetInfo &getTargetInfo() const { return *Target; }
  const TargetInfo *getAuxTargetInfo() const { return AuxTarget; }
  FileManager &getFileManager() const { return FileMgr; }
  SourceManager &getSourceManager() const { return SourceMgr; }

  /// Pop the current lexer/macro exp off the top of the lexer stack.
  ///
  /// This should only be used in situations where the current state of the
  /// top-of-stack lexer is known.
  void RemoveTopOfLexerStack();

  /// Lex the next token for this preprocessor.
  void Lex(Token &Result);

  /// Get the number of tokens processed so far.
  unsigned getTokenCount() const { return TokenCount; }

  /// Get the max number of tokens before issuing a -Wmax-tokens warning.
  unsigned getMaxTokens() const { return MaxTokens; }

  SourceLocation getMaxTokensOverrideLoc() const {
    return MaxTokensOverrideLoc;
  }

  /// Forwarding function for diagnostics.  This emits a diagnostic at
  /// the specified Token's location, translating the token's start
  /// position in the current buffer into a SourcePosition object for rendering.
  DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) const {
    return Diags->Report(Loc, DiagID);
  }

  DiagnosticBuilder Diag(const Token &Tok, unsigned DiagID) const {
    return Diags->Report(Tok.getLocation(), DiagID);
  }

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

  bool InCachingLexMode() const {
    return !CurPPLexer && !CurTokenLexer /*&& !IncludeMacroStack.empty()*/;
  }

  void EnterCachingLexMode();

  void ExitCachingLexMode() {
    if (InCachingLexMode())
      RemoveTopOfLexerStack();
  }

  const Token &PeekAhead(unsigned N);

  /// Return the current lexer being lexed from.
  ///
  /// Note that this ignores any potentially active macro expansions and _Pragma
  /// expansions going on at the time.
  // PreprocessorLexer *getCurrentLexer() const { return CurrLex; };

private:
  /// Add a "macro" context to the top of the include stack,
  /// which will cause the lexer to start returning the specified tokens.
  ///
  /// If \p DisableMacroExpansion is true, tokens lexed from the token stream
  /// will not be subject to further macro expansion. Otherwise, these tokens
  /// will be re-macro-expanded when/if expansion is enabled.
  ///
  /// If \p OwnsTokens is false, this method assumes that the specified stream
  /// of tokens has a permanent owner somewhere, so they do not need to be
  /// copied. If it is true, it assumes the array of tokens is allocated with
  /// \c new[] and the Preprocessor will delete[] it.
  ///
  /// If \p IsReinject the resulting tokens will have Token::IsReinjected flag
  /// set, see the flag documentation for details.
  void EnterTokenStream(const Token *Toks, unsigned NumToks,
                        bool DisableMacroExpansion, bool OwnsTokens,
                        bool IsReinject);

public:
  void EnterTokenStream(std::unique_ptr<Token[]> Toks, unsigned NumToks,
                        bool DisableMacroExpansion, bool IsReinject) {
    EnterTokenStream(Toks.release(), NumToks, DisableMacroExpansion, true,
                     IsReinject);
  }

  void EnterTokenStream(llvm::ArrayRef<Token> Toks, bool DisableMacroExpansion,
                        bool IsReinject) {
    EnterTokenStream(Toks.data(), Toks.size(), DisableMacroExpansion, false,
                     IsReinject);
  }

  /// Enters a token in the token stream to be lexed next.
  ///
  /// If BackTrack() is called afterwards, the token will remain at the
  /// insertion point.
  /// If \p IsReinject is true, resulting token will have Token::IsReinjected
  /// flag set. See the flag documentation for details.
  void EnterToken(const Token &Tok, bool IsReinject) {
    if (LexLevel) {
      // It's not correct in general to enter caching lex mode while in the
      // middle of a nested lexing action.
      auto TokCopy = std::make_unique<Token[]>(1);
      TokCopy[0] = Tok;
      EnterTokenStream(std::move(TokCopy), 1, true, IsReinject);
    } else {
      EnterCachingLexMode();
      assert(IsReinject && "new tokens in the middle of cached stream");
      CachedTokens.insert(CachedTokens.begin() + CachedLexPos, Tok);
    }
  }

  /// Returns true if incremental processing is enabled
  bool isIncrementalProcessingEnabled() const { return IncrementalProcessing; }

  void enableIncrementalProcessing(bool value = true) {
    IncrementalProcessing = value;
  }

private:
  /// Add a lexer to the top of the include stack and
  /// start lexing tokens from it instead of the current buffer.
  void EnterSourceFileWithLexer(Lexer *TheLexer, const DirectoryLookup *Dir);
};
} // namespace latino

#endif // LLVM_LATINO_LEX_PREPROCESSOR_H