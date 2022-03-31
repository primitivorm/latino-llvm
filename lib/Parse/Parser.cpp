#include "latino/Parse/Parser.h"
#include "latino/Sema/Scope.h"
#include "latino/Sema/Sema.h"

#include "llvm/Support/Path.h"

using namespace latino;

Parser::Parser(Preprocessor &pp, Sema &actions, bool skipFunctionBodies)
    : PP(pp), Actions(actions), Diags(PP.getDiagnostics()) {
  Tok.startToken();
  Tok.setKind(tok::eof);
  //   Actions.CurScope = nullptr;
  //   NumCachedScopes = 0;
}

//===----------------------------------------------------------------------===//
// C99 6.9: External Definitions.
//===----------------------------------------------------------------------===//

// Parser::~Parser() {
//   // If we still have scopes active, delete the scope tree.
//   delete getCurScope();
//   Actions.CurScope = nullptr;

//   // Free the scope cache.
//   for (unsigned i = 0, e = NumCachedScopes; i != e; ++i)
//     delete ScopeCache[i];

//   resetPragmaHandles();

//   PP.removeCommentHandler(CommentSemaHandler.get());
//   PP.clearCodeCompletionHandler();

//   DestroyTemplateIds();
// }

/// Parse the first top-level declaration in a translation unit.
///
///   translation-unit:
/// [C]     external-declaration
/// [C]     translation-unit external-declaration
/// [C++]   top-level-declaration-seq[opt]
/// [C++20] global-module-fragment[opt] module-declaration
///                 top-level-declaration-seq[opt] private-module-fragment[opt]
///
/// Note that in C, it is an error if there is no first declaration.
bool Parser::ParseFirstTopLevelDecl(DeclGroupPtrTy &Result) {
  Actions.ActOnStartOfTranslationUnit();

  // C11 6.9p1 says translation units must have at least one top-level
  // declaration. C++ doesn't have this restriction. We also don't want to
  // complain if we have a precompiled header, although technically if the PCH
  // is empty we should still emit the (pedantic) diagnostic.
  bool NoTopLevelDecls = ParseTopLevelDecl(Result, true);
  // if (NoTopLevelDecls && !Actions.getASTContext().getExternalSource() &&
  //     !getLangOpts().CPlusPlus)
  //   Diag(diag::ext_empty_translation_unit);
  return NoTopLevelDecls;
}

/// ParseTopLevelDecl - Parse one top-level declaration, return whatever the
/// action tells us to.  This returns true if the EOF was encountered.
///
///   top-level-declaration:
///           declaration
/// [C++20]   module-import-declaration
bool Parser::ParseTopLevelDecl(DeclGroupPtrTy &Result, bool IsFirstDecl) {
  // DestroyTemplateIdAnnotationsRAIIObj CleanupRAII(*this);

  // Skip over the EOF token, flagging end of previous input for incremental
  // processing
  // if (Lexer.isIncrementalProcessingEnabled() && Tok.is(tok::eof))
  //   ConsumeToken();

  Result = nullptr;
  switch (Tok.getKind()) {
  case tok::kw_exportar:
    switch (NextToken().getKind()) {
    case tok::kw_modulo:
      goto module_decl;

    // Note: no need to handle kw_import here. We only form kw_import under
    // the Modules TS, and in that case 'export import' is parsed as an
    // export-declaration containing an import-declaration.

    // Recognize context-sensitive C++20 'export module' and 'export import'
    // declarations.
    case tok::identifier: {
      IdentifierInfo *II = NextToken().getIdentifierInfo();
      // if ((II == Ident_module || II == Ident_import) &&
      //     GetLookAheadToken(2).isNot(tok::period)) {
      //   if (II == Ident_module)
      //     goto module_decl;
      //   else
      //     goto import_decl;
      // }
      break;
    }

    default:
      break;
    }
    break;

  case tok::kw_modulo:
  module_decl:
    Result = ParseModuleDecl(IsFirstDecl);
    return false;

    // tok::kw_import is handled by ParseExternalDeclaration. (Under the Modules
    // TS, an import can occur within an export block.)
    // import_decl : {
    //   clang::Decl *ImportDecl = ParseModuleImport(clang::SourceLocation());
    //   Result = Actions.ConvertDeclToDeclGroup(ImportDecl);
    //   return false;
    // }

    // case tok::annot_module_include:
    //   Actions.ActOnModuleInclude(
    //       Tok.getLocation(),
    //       reinterpret_cast<clang::Module *>(Tok.getAnnotationValue()));
    //   ConsumeAnnotationToken();
    //   return false;

    // case tok::annot_module_begin:
    //   Actions.ActOnModuleBegin(
    //       Tok.getLocation(),
    //       reinterpret_cast<clang::Module *>(Tok.getAnnotationValue()));
    //   ConsumeAnnotationToken();
    //   return false;

    // case tok::annot_module_end:
    //   Actions.ActOnModuleEnd(Tok.getLocation(),
    //   reinterpret_cast<clang::Module *>(
    //                                                 Tok.getAnnotationValue()));
    //   ConsumeAnnotationToken();
    //   return false;

  case tok::eof:
    // Check whether -fmax-tokens= was reached.
    // if (PP.getMaxTokens() != 0 && PP.getTokenCount() > PP.getMaxTokens()) {
    //   PP.Diag(Tok.getLocation(), clang::diag::warn_max_tokens_total)
    //       << PP.getTokenCount() << PP.getMaxTokens();

    //   clang::SourceLocation OverrideLoc = PP.getMaxTokensOverrideLoc();
    //   if (OverrideLoc.isValid()) {
    //     PP.Diag(OverrideLoc, clang::diag::note_max_tokens_total_override);
    //   }
    // }

    // Actions.SetLateTemplateParser(LateTemplateParserCallback, nullptr, this);
    // if (!PP.isIncrementalProcessingEnabled())
    //   Actions.ActOnEndOfTranslationUnit();
    // return true;

  case tok::identifier:
    // C++2a [basic.link]p3:
    //   A token sequence beginning with 'export[opt] module' or
    //   'export[opt] import' and not immediately followed by '::'
    //   is never interpreted as the declaration of a top-level-declaration.
    // if ((Tok.getIdentifierInfo() == Ident_module ||
    //      Tok.getIdentifierInfo() == Ident_import) &&
    //     NextToken().isNot(tok::period)) {
    //   if (Tok.getIdentifierInfo() == Ident_module)
    //     goto module_decl;
    //   else
    //     goto import_decl;
    // }
    break;

  default:
    break;
  }

  ParsedAttributesWithRange attrs(AttrFactory);
  // MaybeParseCXX11Attributes(attrs);

  // Result = ParseExternalDeclaration(attrs);
  return false;
}

/// Parse a declaration beginning with the 'module' keyword or C++20
/// context-sensitive keyword (optionally preceded by 'export').
///
///   module-declaration:   [Modules TS + P0629R0]
///     'export'[opt] 'module' module-name attribute-specifier-seq[opt] ';'
///
///   global-module-fragment:  [C++2a]
///     'module' ';' top-level-declaration-seq[opt]
///   module-declaration:      [C++2a]
///     'export'[opt] 'module' module-name module-partition[opt]
///            attribute-specifier-seq[opt] ';'
///   private-module-fragment: [C++2a]
///     'module' ':' 'private' ';' top-level-declaration-seq[opt]
Parser::DeclGroupPtrTy Parser::ParseModuleDecl(bool IsFirstDecl) {
  clang::SourceLocation StartLoc = Tok.getLocation();

  Sema::ModuleDeclKind MDK = TryConsumeToken(tok::kw_exportar)
                                 ? Sema::ModuleDeclKind::Interface
                                 : Sema::ModuleDeclKind::Implementation;

  assert(
      (Tok.is(tok::kw_modulo) ||
       (Tok.is(tok::identifier) && Tok.getIdentifierInfo() == Ident_module)) &&
      "not a module declaration");
  clang::SourceLocation ModuleLoc = ConsumeToken();

  // Attributes appear after the module name, not before.
  // FIXME: Suggest moving the attributes later with a fixit.
  // DiagnoseAndSkipCXXAttributes();

  // // Parse a global-module-fragment, if present.
  // if (getLangOpts().CPlusPlusModules && Tok.is(tok::semi)) {
  //   SourceLocation SemiLoc = ConsumeToken();
  //   if (!IsFirstDecl) {
  //     Diag(StartLoc, diag::err_global_module_introducer_not_at_start)
  //         << SourceRange(StartLoc, SemiLoc);
  //     return nullptr;
  //   }
  //   if (MDK == Sema::ModuleDeclKind::Interface) {
  //     Diag(StartLoc, diag::err_module_fragment_exported)
  //         << /*global*/ 0 << FixItHint::CreateRemoval(StartLoc);
  //   }
  //   return Actions.ActOnGlobalModuleFragmentDecl(ModuleLoc);
  // }

  // // Parse a private-module-fragment, if present.
  // if (getLangOpts().CPlusPlusModules && Tok.is(tok::colon) &&
  //     NextToken().is(tok::kw_private)) {
  //   if (MDK == Sema::ModuleDeclKind::Interface) {
  //     Diag(StartLoc, diag::err_module_fragment_exported)
  //         << /*private*/ 1 << FixItHint::CreateRemoval(StartLoc);
  //   }
  //   ConsumeToken();
  //   SourceLocation PrivateLoc = ConsumeToken();
  //   DiagnoseAndSkipCXX11Attributes();
  //   ExpectAndConsumeSemi(diag::err_private_module_fragment_expected_semi);
  //   return Actions.ActOnPrivateModuleFragmentDecl(ModuleLoc, PrivateLoc);
  // }

  llvm::SmallVector<std::pair<IdentifierInfo *, clang::SourceLocation>, 2> Path;

  // if (ParseModuleName(ModuleLoc, Path, /*IsImport*/ false))
  //   return nullptr;

  // // Parse the optional module-partition.
  // if (Tok.is(tok::colon)) {
  //   SourceLocation ColonLoc = ConsumeToken();
  //   SmallVector<std::pair<IdentifierInfo *, SourceLocation>, 2> Partition;
  //   if (ParseModuleName(ModuleLoc, Partition, /*IsImport*/ false))
  //     return nullptr;

  //   // FIXME: Support module partition declarations.
  //   Diag(ColonLoc, diag::err_unsupported_module_partition)
  //       << SourceRange(ColonLoc, Partition.back().second);
  //   // Recover by parsing as a non-partition.
  // }

  // // We don't support any module attributes yet; just parse them and
  // diagnose.
  //   ParsedAttributesWithRange Attrs(AttrFactory);
  //   MaybeParseCXX11Attributes(Attrs);
  //   ProhibitCXX11Attributes(Attrs, diag::err_attribute_not_module_attr);

  //   ExpectAndConsumeSemi(diag::err_module_expected_semi);

  return Actions.ActOnModuleDecl(StartLoc, ModuleLoc, MDK, /* Path ,*/
                                 IsFirstDecl);
}
