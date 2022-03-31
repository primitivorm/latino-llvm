#include "clang/Parse/RAIIObjectsForParser.h"
#include "clang/Sema/DeclSpec.h"

#include "latino/Parse/Parser.h"
#include "latino/Sema/Scope.h"

using namespace latino;

/// Parse a standalone statement (for instance, as the body of an 'if',
/// 'while', or 'for').
clang::StmtResult Parser::ParseStatement(clang::SourceLocation *TrailingElseLoc,
                                         ParsedStmtContext StmtCtx) {
  clang::StmtResult Res;
  // We may get back a null statement if we found a #pragma. Keep going until
  // we get an actual statement.
  do {
    StmtVector Stmts;
    Res = ParseStatementOrDeclaration(Stmts, StmtCtx, TrailingElseLoc);
  } while (!Res.isInvalid() && !Res.get());

  return Res;
}

/// ParseStatementOrDeclaration - Read 'statement' or 'declaration'.
///       StatementOrDeclaration:
///         statement
///         declaration
///
///       statement:
///         labeled-statement
///         compound-statement
///         expression-statement
///         selection-statement
///         iteration-statement
///         jump-statement
/// [C++]   declaration-statement
/// [C++]   try-block
/// [MS]    seh-try-block
/// [OBC]   objc-throw-statement
/// [OBC]   objc-try-catch-statement
/// [OBC]   objc-synchronized-statement
/// [GNU]   asm-statement
/// [OMP]   openmp-construct             [TODO]
///
///       labeled-statement:
///         identifier ':' statement
///         'case' constant-expression ':' statement
///         'default' ':' statement
///
///       selection-statement:
///         if-statement
///         switch-statement
///
///       iteration-statement:
///         while-statement
///         do-statement
///         for-statement
///
///       expression-statement:
///         expression[opt] ';'
///
///       jump-statement:
///         'goto' identifier ';'
///         'continue' ';'
///         'break' ';'
///         'return' expression[opt] ';'
/// [GNU]   'goto' '*' expression ';'
///
/// [OBC] objc-throw-statement:
/// [OBC]   '@' 'throw' expression ';'
/// [OBC]   '@' 'throw' ';'
///

clang::StmtResult
Parser::ParseStatementOrDeclaration(StmtVector &Stmts,
                                    ParsedStmtContext StmtCtx,
                                    clang::SourceLocation *TrailingElseLoc) {
  // clang::ParenBraceBracketBalancer BalancerRAIIObj(*this);

  ParsedAttributesWithRange Attrs(AttrFactory);
  // MaybeParseCXX11Attributes(Attrs, nullptr, /*MightBeObjCMessageSend*/ true);
  // if (!MaybeParseOpenCLUnrollHintAttribute(Attrs))
  //   return StmtError();

  clang::StmtResult Res = ParseStatementOrDeclarationAfterAttributes(
      Stmts, StmtCtx, TrailingElseLoc, Attrs);

  // MaybeDestroyTemplateIds();
  assert((Attrs.empty() || Res.isInvalid() || Res.isUsable()) &&
         "attributes on empty statement");

  if (Attrs.empty() || Res.isInvalid())
    return Res;

  return Actions.ProcessStmtAttributes(Res.get(), Attrs, Attrs.Range);
}

clang::StmtResult Parser::ParseStatementOrDeclarationAfterAttributes(
    StmtVector &Stmts, ParsedStmtContext StmtCtx,
    clang::SourceLocation *TrailingElseLoc, ParsedAttributesWithRange &Attrs) {
  const char *SemiError = nullptr;
  clang::StmtResult Res;

  // Cases in this switch statement should fall through if the parser expects
  // the token to end in a semicolon (in which case SemiError should be set),
  // or they directly 'return;' if not.
  // Retry:
  tok::TokenKind Kind = Tok.getKind();
  clang::SourceLocation AtLoc;
  switch (Kind) {
  case tok::identifier: {
    Token Next = NextToken();
    if (Next.is(tok::colon)) { // C99 6.8.1: labeled-statement
      // identifier ':' statement
      return ParseLabeledStatement(Attrs, StmtCtx);
    }

    // // Look up the identifier, and typo-correct it to a keyword if it's not
    // // found.
    // if (Next.isNot(tok::coloncolon)) {
    //   // Try to limit which sets of keywords should be included in typo
    //   // correction based on what the next token is.
    //   StatementFilterCCC CCC(Next);
    //   if (TryAnnotateName(&CCC) == ANK_Error) {
    //     // Handle errors here by skipping up to the next semicolon or '}',
    //     and
    //     // eat the semicolon if that's what stopped us.
    //     SkipUntil(tok::r_brace, StopAtSemi | StopBeforeMatch);
    //     if (Tok.is(tok::semi))
    //       ConsumeToken();
    //     return StmtError();
    //   }

    //   // If the identifier was typo-corrected, try again.
    //   if (Tok.isNot(tok::identifier))
    //     goto Retry;
    // }

    // Fall through
    LLVM_FALLTHROUGH;
  }
  default: {
    // if ((getLangOpts().CPlusPlus || getLangOpts().MicrosoftExt ||
    //      (StmtCtx & ParsedStmtContext::AllowDeclarationsInC) !=
    //          ParsedStmtContext()) &&
    //     (GNUAttributeLoc.isValid() || isDeclarationStatement())) {
    //   SourceLocation DeclStart = Tok.getLocation(), DeclEnd;
    //   DeclGroupPtrTy Decl;
    //   if (GNUAttributeLoc.isValid()) {
    //     DeclStart = GNUAttributeLoc;
    //     Decl = ParseDeclaration(DeclaratorContext::BlockContext, DeclEnd,
    //     Attrs,
    //                             &GNUAttributeLoc);
    //   } else {
    //     Decl =
    //         ParseDeclaration(DeclaratorContext::BlockContext, DeclEnd,
    //         Attrs);
    //   }
    //   if (Attrs.Range.getBegin().isValid())
    //     DeclStart = Attrs.Range.getBegin();
    //   return Actions.ActOnDeclStmt(Decl, DeclStart, DeclEnd);
    // }

    if (Tok.is(tok::r_brace)) {
      // Diag(Tok, diag::err_expected_statement);
      return clang::StmtError();
    }

    return ParseExprStatement(StmtCtx);
  }

  case tok::kw_caso: // C99 6.8.1: labeled-statement
    return ParseCaseStatement(StmtCtx);

  case tok::kw_otro: // C99 6.8.1: labeled-statement
    return ParseDefaultStatement(StmtCtx);

    // case tok::l_brace:
    //   return ParseCompoundStatement(StmtCtx);

    // case tok::semi: { // C99 6.8.3p3: expression[opt] ';'
    //   bool HasLeadingEmptyMacro = Tok.hasLeadingEmptyMacro();
    //   return Actions.ActOnNullStmt(ConsumeToken(), HasLeadingEmptyMacro);
    // }

  case tok::kw_si: // C99 6.8.4.1: if-statement
    return ParseIfStatement(TrailingElseLoc);

  case tok::kw_elegir: // C99 6.8.4.2: switch-statement
    return ParseSwitchStatement(TrailingElseLoc);

  case tok::kw_mientras: // C99 6.8.5.1: while-statement
    return ParseWhileStatement(TrailingElseLoc);

  case tok::kw_desde: // C99 6.8.5.3: for-statement
    return ParseForStatement(TrailingElseLoc);

  case tok::kw_ir: // C99 6.8.6.1: goto-statement
    Res = ParseGotoStatement();
    // SemiError = "goto";
    break;

  case tok::kw_continuar: // C99 6.8.6.2: continue-statement
    Res = ParseContinueStatement();
    // SemiError = "continue";
    break;

  case tok::kw_ret: // C99 6.8.6.4: return-statement
    Res = ParseReturnStatement();
    SemiError = "return";
    break;

  case tok::kw_intentar: // C++ 15: try-block
    return ParseCXXTryBlock();
  }

  // If we reached this code, the statement must end in a semicolon.
  // if (!TryConsumeToken(tok::semi) && !Res.isInvalid()) {
  //   // If the result was valid, then we do want to diagnose this.  Use
  //   // ExpectAndConsume to emit the diagnostic, even though we know it won't
  //   // succeed.
  //   ExpectAndConsume(tok::semi, diag::err_expected_semi_after_stmt,
  //   SemiError);
  //   // Skip until we see a } or ;, but don't eat it.
  //   SkipUntil(tok::r_brace, StopAtSemi | StopBeforeMatch);
  // }

  return Res;
}

/// Parse an expression statement.
clang::StmtResult Parser::ParseExprStatement(ParsedStmtContext StmtCtx) {
  // If a case keyword is missing, this is where it should be inserted.
  Token OldToken = Tok;

  ExprStatementTokLoc = Tok.getLocation();

  // expression[opt] ';'
  clang::ExprResult Expr(ParseExpression());
  if (Expr.isInvalid()) {
    // If the expression is invalid, skip ahead to the next semicolon or '}'.
    // Not doing this opens us up to the possibility of infinite loops if
    // ParseExpression does not consume any tokens.
    // SkipUntil(tok::r_brace, StopAtSemi | StopBeforeMatch);
    if (Tok.is(tok::semi))
      ConsumeToken();
    return Actions.ActOnExprStmtError();
  }

  if (Tok.is(tok::colon) && getCurScope()->isSwitchScope() &&
      Actions.CheckCaseExpression(Expr.get())) {
    // If a constant expression is followed by a colon inside a switch block,
    // suggest a missing case keyword.
    // Diag(OldToken, diag::err_expected_case_before_expression)
    //     << FixItHint::CreateInsertion(OldToken.getLocation(), "case ");

    // Recover parsing as a case statement.
    return ParseCaseStatement(StmtCtx, /*MissingCase=*/true, Expr);
  }

  // Otherwise, eat the semicolon.
  // ExpectAndConsumeSemi(diag::err_expected_semi_after_expr);
  return handleExprStmt(Expr, StmtCtx);
}

/// ParseLabeledStatement - We have an identifier and a ':' after it.
///
///       labeled-statement:
///         identifier ':' statement
/// [GNU]   identifier ':' attributes[opt] statement
///
clang::StmtResult
Parser::ParseLabeledStatement(ParsedAttributesWithRange &attrs,
                              ParsedStmtContext StmtCtx) {
  assert(Tok.is(tok::identifier) && Tok.getIdentifierInfo() &&
         "Not an identifier!");

  // The substatement is always a 'statement', not a 'declaration', but is
  // otherwise in the same context as the labeled-statement.
  // StmtCtx &= ~ParsedStmtContext::AllowDeclarationsInC;

  Token IdentTok = Tok; // Save the whole token.
  ConsumeToken();       // eat the identifier.

  assert(Tok.is(tok::colon) && "Not a label!");

  // identifier ':' statement
  clang::SourceLocation ColonLoc = ConsumeToken();

  // Read label attributes, if present.
  clang::StmtResult SubStmt;

  // // Read label attributes, if present.
  // if (Tok.is(tok::kw___attribute)) {
  //   ParsedAttributesWithRange TempAttrs(AttrFactory);
  //   ParseGNUAttributes(TempAttrs);

  //   // In C++, GNU attributes only apply to the label if they are followed by
  //   a
  //   // semicolon, to disambiguate label attributes from attributes on a
  //   labeled
  //   // declaration.
  //   //
  //   // This doesn't quite match what GCC does; if the attribute list is empty
  //   // and followed by a semicolon, GCC will reject (it appears to parse the
  //   // attributes as part of a statement in that case). That looks like a
  //   bug. if (!getLangOpts().CPlusPlus || Tok.is(tok::semi))
  //     attrs.takeAllFrom(TempAttrs);
  //   else if (isDeclarationStatement()) {
  //     StmtVector Stmts;
  //     // FIXME: We should do this whether or not we have a declaration
  //     // statement, but that doesn't work correctly (because
  //     ProhibitAttributes
  //     // can't handle GNU attributes), so only call it in the one case where
  //     // GNU attributes are allowed.
  //     SubStmt = ParseStatementOrDeclarationAfterAttributes(Stmts, StmtCtx,
  //                                                          nullptr,
  //                                                          TempAttrs);
  //     if (!TempAttrs.empty() && !SubStmt.isInvalid())
  //       SubStmt = Actions.ProcessStmtAttributes(SubStmt.get(), TempAttrs,
  //                                               TempAttrs.Range);
  //   } else {
  //     Diag(Tok, diag::err_expected_after) << "__attribute__" << tok::semi;
  //   }
  // }

  // If we've not parsed a statement yet, parse one now.
  if (!SubStmt.isInvalid() && !SubStmt.isUsable())
    SubStmt = ParseStatement(nullptr, StmtCtx);

  // Broken substmt shouldn't prevent the label from being added to the AST.
  if (SubStmt.isInvalid())
    SubStmt = Actions.ActOnNullStmt(ColonLoc);

  clang::LabelDecl *LD = Actions.LookupOrCreateLabel(
      IdentTok.getIdentifierInfo(), IdentTok.getLocation());

  // Actions.ProcessDeclAttributeList(Actions.CurScope, LD, attrs);
  // attrs.clear();

  return Actions.ActOnLabelStmt(IdentTok.getLocation(), LD, ColonLoc,
                                SubStmt.get());
}

clang::StmtResult Parser::handleExprStmt(clang::ExprResult E,
                                         ParsedStmtContext StmtCtx) {
  bool IsStmtExprResult = false;
  if ((StmtCtx & ParsedStmtContext::InStmtExpr) != ParsedStmtContext()) {
    // For GCC compatibility we skip past NullStmts.
    unsigned LookAhead = 0;
    while (GetLookAheadToken(LookAhead).is(tok::semi)) {
      ++LookAhead;
    }
    // Then look to see if the next two tokens close the statement expression;
    // if so, this expression statement is the last statement in a statment
    // expression.
    IsStmtExprResult = GetLookAheadToken(LookAhead).is(tok::r_brace) &&
                       GetLookAheadToken(LookAhead + 1).is(tok::r_paren);
  }

  if (IsStmtExprResult)
    E = Actions.ActOnStmtExprResult(E);
  return Actions.ActOnExprStmt(E, /*DiscardedValue=*/!IsStmtExprResult);
}
