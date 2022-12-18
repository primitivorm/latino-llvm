#include "latino/Lex/Preprocessor.h"
#include "latino/Basic/Builtins.h"
#include "latino/Basic/LLVM.h"
#include "latino/Basic/TargetInfo.h"
#include "latino/Lex/HeaderSearch.h"
#include "latino/Lex/ModuleLoader.h"

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

Preprocessor::Preprocessor(std::shared_ptr<clang::PreprocessorOptions> PPOpts,
                           DiagnosticsEngine &diags, LangOptions &opts,
                           SourceManager &SM, HeaderSearch &Headers,
                           ModuleLoader &TheModuleLoader,
                           IdentifierInfoLookup *IILookup, bool OwnsHeaders,
                           TranslationUnitKind TUKind)
    : PPOpts(std::move(PPOpts)), Diags(&diags), LangOpts(opts),
      FileMgr(Headers.getFileMgr()), SourceMgr(SM), /*HeaderInfo(Headers),*/
      TheModuleLoader(TheModuleLoader), Identifiers(IILookup), TUKind(TUKind) {
  MaxTokens = LangOpts.MaxTokens;

  BuiltinInfo = std::make_unique<latino::Builtin::Context>();
}

Preprocessor::~Preprocessor() {
  // TODO: Pending implementation
}

void Preprocessor::Initialize(const TargetInfo &Target,
                              const TargetInfo *AuxTarget) {
  assert((!this->Target || this->Target == &Target) &&
         "Invalid override target information.");
  this->Target = &Target;

  assert((!this->AuxTarget || this->AuxTarget == AuxTarget) &&
         "Ivalid override target information.");
  this->AuxTarget = AuxTarget;

  // Initialize information about built-ins.
  BuiltinInfo->InitializeTarget(Target, AuxTarget);
  // HeaderInfo.setTarget(Target);

  // Populate the identifier table with info about keywords for the current
  // language.
  Identifiers.AddKeywords(LangOpts);
}

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
  FileID MainFileID = SourceMgr.getMainFileID();

  // If MainFileID is loaded it means we loaded an AST file, no need to enter
  // a main file.
  if (!SourceMgr.isLoadedFileID(MainFileID)) {
    // Enter the main file source buffer.
    EnterSourceFile(MainFileID, nullptr, SourceLocation());

    // If we've been asked to skip bytes in the main file (e.g., as part of a
    // precompiled preamble), do so now.
    // if (SkipMainFilePreamble.first > 0)
    //   CurLexer->SetByteOffset(SkipMainFilePreamble.first,
    //                           SkipMainFilePreamble.second);
  }

  // Preprocess Predefines to populate the initial preprocessor state.
  // std::unique_ptr<llvm::MemoryBuffer> SB =
  //   llvm::MemoryBuffer::getMemBufferCopy(Predefines, "<built-in>");
  // assert(SB && "Cannot create predefined source buffer");
  // FileID FID = SourceMgr.createFileID(std::move(SB));
  // assert(FID.isValid() && "Could not create FileID for predefines?");
  // setPredefinesFileID(FID);

  // // Start parsing the predefines.
  // EnterSourceFile(FID, nullptr, SourceLocation());

  // if (!PPOpts->PCHThroughHeader.empty()) {
  //   // Lookup and save the FileID for the through header. If it isn't found
  //   // in the search path, it's a fatal error.
  //   const DirectoryLookup *CurDir;
  //   Optional<FileEntryRef> File = LookupFile(
  //       SourceLocation(), PPOpts->PCHThroughHeader,
  //       /*isAngled=*/false, /*FromDir=*/nullptr, /*FromFile=*/nullptr,
  //       CurDir,
  //       /*SearchPath=*/nullptr, /*RelativePath=*/nullptr,
  //       /*SuggestedModule=*/nullptr, /*IsMapped=*/nullptr,
  //       /*IsFrameworkFound=*/nullptr);
  //   if (!File) {
  //     Diag(SourceLocation(), diag::err_pp_through_header_not_found)
  //         << PPOpts->PCHThroughHeader;
  //     return;
  //   }
  //   setPCHThroughHeaderFileID(
  //       SourceMgr.createFileID(*File, SourceLocation(), SrcMgr::C_User));
  // }

  // // Skip tokens from the Predefines and if needed the main file.
  // if ((usingPCHWithThroughHeader() && SkippingUntilPCHThroughHeader) ||
  //     (usingPCHWithPragmaHdrStop() && SkippingUntilPragmaHdrStop))
  //   SkipTokensWhileUsingPCH();
}

void Preprocessor::Lex(Token &Result) {
  ++LexLevel;
  // We loop here until a lex function returns a token; this avoids recursion.
  bool ReturnedToken;
  do {
    switch (CurLexerKind) {
    case CLK_Lexer:
      ReturnedToken = CurLexer->Lex(Result);
      break;
    }
  } while (!ReturnedToken);

  // if (Result.is(tok::unknown) && TheModuleLoader.HadFatalFailure)
  //   return;

  // if (Result.is(tok::code_completion) && Result.getIdentifierInfo()) {
  //   // Remember the identifier before code completion token.
  //   setCodeCompletionIdentifierInfo(Result.getIdentifierInfo());
  //   setCodeCompletionTokenRange(Result.getLocation(), Result.getEndLoc());
  //   // Set IdenfitierInfo to null to avoid confusing code that handles both
  //   // identifiers and completion tokens.
  //   Result.setIdentifierInfo(nullptr);
  // }

  // // Update ImportSeqState to track our position within a C++20 import-seq
  // // if this token is being produced as a result of phase 4 of translation.
  // if (getLangOpts().CPlusPlusModules && LexLevel == 1 &&
  //     !Result.getFlag(Token::IsReinjected)) {
  //   switch (Result.getKind()) {
  //   case tok::l_paren:
  //   case tok::l_square:
  //   case tok::l_brace:
  //     ImportSeqState.handleOpenBracket();
  //     break;
  //   case tok::r_paren:
  //   case tok::r_square:
  //     ImportSeqState.handleCloseBracket();
  //     break;
  //   case tok::r_brace:
  //     ImportSeqState.handleCloseBrace();
  //     break;
  //   case tok::semi:
  //     ImportSeqState.handleSemi();
  //     break;
  //   case tok::header_name:
  //   case tok::annot_header_unit:
  //     ImportSeqState.handleHeaderName();
  //     break;
  //   case tok::kw_export:
  //     ImportSeqState.handleExport();
  //     break;
  //   case tok::identifier:
  //     if (Result.getIdentifierInfo()->isModulesImport()) {
  //       ImportSeqState.handleImport();
  //       if (ImportSeqState.afterImportSeq()) {
  //         ModuleImportLoc = Result.getLocation();
  //         ModuleImportPath.clear();
  //         ModuleImportExpectsIdentifier = true;
  //         CurLexerKind = CLK_LexAfterModuleImport;
  //       }
  //       break;
  //     }
  //     LLVM_FALLTHROUGH;
  //   default:
  //     ImportSeqState.handleMisc();
  //     break;
  //   }
  // }

  LastTokenWasAt = Result.is(tok::at);
  --LexLevel;

  if (LexLevel == 0 && !Result.getFlag(Token::IsReinjected)) {
    ++TokenCount;
    if (OnToken)
      OnToken(Result);
  }
}

ModuleLoader::~ModuleLoader() = default;