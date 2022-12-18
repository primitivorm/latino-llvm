
#include "latino/Sema/Designator.h"
#include "latino/Sema/Scope.h"

#include "latino/Basic/TokenKinds.h"
#include "latino/Parse/Parser.h"
#include "latino/Parse/RAIIObjectsForParser.h"
#include "latino/Sema/Ownership.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"

using namespace latino;

/// ParseBraceInitializer - Called when parsing an initializer that has a
/// leading open brace.
///
///       initializer: [C99 6.7.8]
///         '{' initializer-list '}'
///         '{' initializer-list ',' '}'
/// [GNU]   '{' '}'
///
///       initializer-list:
///         designation[opt] initializer ...[opt]
///         initializer-list ',' designation[opt] initializer ...[opt]
///
ExprResult Parser::ParseBraceInitializer() {
  // InMessageExpressionRAIIObject InMessage(*this, false);

  BalancedDelimiterTracker T(*this, tok::l_brace);
  T.consumeOpen();
  SourceLocation LBraceLoc = T.getOpenLocation();

  /// InitExprs - This is the actual list of expressions contained in the
  /// initializer.
  ExprVector InitExprs;

  // if (Tok.is(tok::r_brace)) {
  //   // Diag(LBraceLoc, diag::ext_gnu_empty_initializer);
  //   // Match the '}'.
  //   return Actions.ActOnInitList(LBraceLoc, llvm::None, ConsumeBrace());
  // }

  // Enter an appropriate expression evaluation context for an initializer list.
  // clang::EnterExpressionEvaluationContext EnterContext(
  //     Actions, clang::EnterExpressionEvaluationContext::InitList);

  bool InitExprsOk = true;
  // auto CodeCompletionDesignation = [&](const Designation &D) {
  //   Actions.CodeCompleteDesignator(PreferredType.get(T.getOpenLocation()),
  //                                  InitExprs, D);
  // };

  while (1) {
    // // Handle Microsoft __if_exists/if_not_exists if necessary.
    // if (getLangOpts().MicrosoftExt &&
    //     (Tok.is(tok::kw___if_exists) || Tok.is(tok::kw___if_not_exists))) {
    //   if (ParseMicrosoftIfExistsBraceInitializer(InitExprs, InitExprsOk)) {
    //     if (Tok.isNot(tok::comma))
    //       break;
    //     ConsumeToken();
    //   }
    //   if (Tok.is(tok::r_brace))
    //     break;
    //   continue;
    // }

    // Parse: designation[opt] initializer

    // If we know that this cannot be a designation, just parse the nested
    // initializer directly.
    ExprResult SubElt;
    if (MayBeDesignationStart())
      // SubElt =
      // ParseInitializerWithPotentialDesignator(CodeCompleteDesignator);
      ;
    else
      SubElt = ParseInitializer();

    // if (Tok.is(tok::ellipsis))
    //   SubElt = Actions.ActOnPackExpansion(SubElt.get(), ConsumeToken());

    // SubElt = Actions.CorrectDelayedTyposInExpr(SubElt.get());

    // If we couldn't parse the subelement, bail out.
    if (SubElt.isUsable()) {
      InitExprs.push_back(SubElt.get());
    } else {
      InitExprsOk = false;

      // We have two ways to try to recover from this error: if the code looks
      // grammatically ok (i.e. we have a comma coming up) try to continue
      // parsing the rest of the initializer.  This allows us to emit
      // diagnostics for later elements that we find.  If we don't see a comma,
      // assume there is a parse error, and just skip to recover.
      // FIXME: This comment doesn't sound right. If there is a r_brace
      // immediately, it can't be an error, since there is no other way of
      // leaving this loop except through this if.
      // if (Tok.isNot(tok::comma)) {
      //   SkipUntil(tok::r_brace, StopBeforeMatch);
      //   break;
      // }
    }

    // If we don't have a comma continued list, we're done.
    if (Tok.isNot(tok::comma))
      break;

    // TODO: save comma locations if some client cares.
    ConsumeToken();

    // Handle trailing comma.
    if (Tok.is(tok::r_brace))
      break;
  }

  // bool closed = !T.consumeClose();
  bool closed = true;

  // if (InitExprsOk && closed)
  //   return Actions.ActOnInitList(LBraceLoc, InitExprs, T.getCloseLocation());

  return ExprError(); // an error occurred.
}