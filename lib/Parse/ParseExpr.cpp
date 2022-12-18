#include "latino/Sema/DeclSpec.h"

#include "latino/Sema/Sema.h"

#include "latino/Parse/Parser.h"
#include "latino/Sema/Scope.h"

#include "llvm/ADT/SmallVector.h"

using namespace latino;

/// Simple precedence-based parser for binary/ternary operators.
///
/// Note: we diverge from the C99 grammar when parsing the assignment-expression
/// production.  C99 specifies that the LHS of an assignment operator should be
/// parsed as a unary-expression, but consistency dictates that it be a
/// conditional-expession.  In practice, the important thing here is that the
/// LHS of an assignment has to be an l-value, which productions between
/// unary-expression and conditional-expression don't produce.  Because we want
/// consistency, we parse the LHS as a conditional-expression, then check for
/// l-value-ness in semantic analysis stages.
///
/// \verbatim
///       pm-expression: [C++ 5.5]
///         cast-expression
///         pm-expression '.*' cast-expression
///         pm-expression '->*' cast-expression
///
///       multiplicative-expression: [C99 6.5.5]
///     Note: in C++, apply pm-expression instead of cast-expression
///         cast-expression
///         multiplicative-expression '*' cast-expression
///         multiplicative-expression '/' cast-expression
///         multiplicative-expression '%' cast-expression
///
///       additive-expression: [C99 6.5.6]
///         multiplicative-expression
///         additive-expression '+' multiplicative-expression
///         additive-expression '-' multiplicative-expression
///
///       shift-expression: [C99 6.5.7]
///         additive-expression
///         shift-expression '<<' additive-expression
///         shift-expression '>>' additive-expression
///
///       compare-expression: [C++20 expr.spaceship]
///         shift-expression
///         compare-expression '<=>' shift-expression
///
///       relational-expression: [C99 6.5.8]
///         compare-expression
///         relational-expression '<' compare-expression
///         relational-expression '>' compare-expression
///         relational-expression '<=' compare-expression
///         relational-expression '>=' compare-expression
///
///       equality-expression: [C99 6.5.9]
///         relational-expression
///         equality-expression '==' relational-expression
///         equality-expression '!=' relational-expression
///
///       AND-expression: [C99 6.5.10]
///         equality-expression
///         AND-expression '&' equality-expression
///
///       exclusive-OR-expression: [C99 6.5.11]
///         AND-expression
///         exclusive-OR-expression '^' AND-expression
///
///       inclusive-OR-expression: [C99 6.5.12]
///         exclusive-OR-expression
///         inclusive-OR-expression '|' exclusive-OR-expression
///
///       logical-AND-expression: [C99 6.5.13]
///         inclusive-OR-expression
///         logical-AND-expression '&&' inclusive-OR-expression
///
///       logical-OR-expression: [C99 6.5.14]
///         logical-AND-expression
///         logical-OR-expression '||' logical-AND-expression
///
///       conditional-expression: [C99 6.5.15]
///         logical-OR-expression
///         logical-OR-expression '?' expression ':' conditional-expression
/// [GNU]   logical-OR-expression '?' ':' conditional-expression
/// [C++] the third operand is an assignment-expression
///
///       assignment-expression: [C99 6.5.16]
///         conditional-expression
///         unary-expression assignment-operator assignment-expression
/// [C++]   throw-expression [C++ 15]
///
///       assignment-operator: one of
///         = *= /= %= += -= <<= >>= &= ^= |=
///
///       expression: [C99 6.5.17]
///         assignment-expression ...[opt]
///         expression ',' assignment-expression ...[opt]
/// \endverbatim

ExprResult Parser::ParseExpression(TypeCastState isTypeCast) {
  ExprResult LHS(ParseAssignmentExpression(isTypeCast));
  return ParseRHSOfBinaryExpression(LHS, prec::Comma);
}

bool Parser::isNotExpressionStart() {
  tok::TokenKind K = Tok.getKind();
  if (K == tok::l_brace || K == tok::r_brace || K == tok::kw_desde ||
      K == tok::kw_mientras || K == tok::kw_si || K == tok::kw_sino ||
      K == tok::kw_ir || K == tok::kw_intentar)
    return true;
  // If this is a decl-specifier, we can't be at the start of an expression.
  return isKnownToBeDeclarationSpecifier();
}

/// Parse an expr that doesn't include (top-level) commas.
ExprResult Parser::ParseAssignmentExpression(TypeCastState isTypeCast) {
  //     if (Tok.is(tok::code_completion)) {
  //     Actions.CodeCompleteExpression(getCurScope(),
  //                                    PreferredType.get(Tok.getLocation()));
  //     cutOffParsing();
  //     return ExprError();
  //   }

  if (Tok.is(tok::kw_intentar))
    return ParseThrowExpression();
  //   if (Tok.is(tok::kw_co_yield))
  //     return ParseCoyieldExpression();

  ExprResult LHS = ParseCastExpression(
      AnyCastExpr, /*isAddressOfOperand=*/false, isTypeCast);

  return ParseRHSOfBinaryExpression(LHS, prec::Assignment);
}

bool Parser::isFoldOperator(prec::Level Level) const {
  return Level > prec::Unknown && Level != prec::Conditional &&
         Level != prec::Spaceship;
}

bool Parser::isFoldOperator(tok::TokenKind Kind) const {
  return isFoldOperator(
      getBinOpPrecedence(Kind, /*GreaterThanIsOperator*/ true, true));
}

/// Parse a binary expression that starts with \p LHS and has a
/// precedence of at least \p MinPrec.
ExprResult Parser::ParseRHSOfBinaryExpression(ExprResult LHS,
                                              prec::Level MinPrec) {
  prec::Level NextTokPrec =
      getBinOpPrecedence(Tok.getKind(), GreaterThanIsOperator,
                         /*CPlusPlus11*/ true);

  SourceLocation ColonLoc;

  // auto SavedType = PreferredType;
  while (1) {
    // Every iteration may rely on a preferred type for the whole expression.
    // PreferredType = SavedType;

    // If this token has a lower precedence than we are allowed to parse (e.g.
    // because we are called recursively, or because the token is not a binop),
    // then we are done!
    if (NextTokPrec < MinPrec)
      return LHS;

    // Consume the operator, saving the operator token for error reporting.
    Token OpToken = Tok;
    ConsumeToken();

    // if (OpToken.is(tok::caretcaret)) {
    //   return ExprError(Diag(Tok, diag::err_opencl_logical_exclusive_or));
    // }

    // If we're potentially in a template-id, we may now be able to determine
    // whether we're actually in one or not.
    // if (OpToken.isOneOf(tok::comma, tok::greater, tok::greatergreater,
    //                     tok::greatergreatergreater) &&
    //     checkPotentialAngleBracketDelimiter(OpToken))
    //   return ExprError();

    // Bail out when encountering a comma followed by a token which can't
    // possibly be the start of an expression. For instance:
    //   int f() { return 1, }
    // We can't do this before consuming the comma, because
    // isNotExpressionStart() looks at the token stream.
    if (OpToken.is(tok::comma) && isNotExpressionStart()) {
      PP.EnterToken(Tok, /*IsReinject*/ true);
      Tok = OpToken;
      return LHS;
    }

    // If the next token is an ellipsis, then this is a fold-expression. Leave
    // it alone so we can handle it in the paren expression.
    if (isFoldOperator(NextTokPrec) && Tok.is(tok::ellipsis)) {
      // FIXME: We can't check this via lookahead before we consume the token
      // because that tickles a lexer bug.
      PP.EnterToken(Tok, /*IsReinject*/ true);
      Tok = OpToken;
      return LHS;
    }

    // In Objective-C++, alternative operator tokens can be used as keyword args
    // in message expressions. Unconsume the token so that it can reinterpreted
    // as an identifier in ParseObjCMessageExpressionBody. i.e., we support:
    //   [foo meth:0 and:0];
    //   [foo not_eq];
    if (/*getLangOpts().ObjC && getLangOpts().CPlusPlus &&*/
        Tok.isOneOf(tok::colon, tok::r_square) &&
        OpToken.getIdentifierInfo() != nullptr) {
      PP.EnterToken(Tok, /*IsReinject*/ true);
      Tok = OpToken;
      return LHS;
    }

    // Special case handling for the ternary operator.
    ExprResult TernaryMiddle(true);
    if (NextTokPrec == prec::Conditional) {
      if (/*getLangOpts().CPlusPlus11 &&*/ Tok.is(tok::l_brace)) {
        // Parse a braced-init-list here for error recovery purposes.
        SourceLocation BraceLoc = Tok.getLocation();
        TernaryMiddle = ParseBraceInitializer();
        if (!TernaryMiddle.isInvalid()) {
          // Diag(BraceLoc, diag::err_init_list_bin_op)
          //     << /*RHS*/ 1 << PP.getSpelling(OpToken)
          //     << Actions.getExprRange(TernaryMiddle.get());
          TernaryMiddle = ExprError();
        }
      } else if (Tok.isNot(tok::colon)) {
        // Don't parse FOO:BAR as if it were a typo for FOO::BAR.
        // ColonProtectionRAIIObject X(*this);

        // Handle this production specially:
        //   logical-OR-expression '?' expression ':' conditional-expression
        // In particular, the RHS of the '?' is 'expression', not
        // 'logical-OR-expression' as we might expect.
        TernaryMiddle = ParseExpression();
      } /* else {
        // Special case handling of "X ? Y : Z" where Y is empty:
        //   logical-OR-expression '?' ':' conditional-expression   [GNU]
        TernaryMiddle = nullptr;
        // Diag(Tok, diag::ext_gnu_conditional_expr);
      }*/

      // if (TernaryMiddle.isInvalid()) {
      //   Actions.CorrectDelayedTyposInExpr(LHS);
      //   LHS = ExprError();
      //   TernaryMiddle = nullptr;
      // }

      if (!TryConsumeToken(tok::colon, ColonLoc)) {
        // Otherwise, we're missing a ':'.  Assume that this was a typo that
        // the user forgot. If we're not in a macro expansion, we can suggest
        // a fixit hint. If there were two spaces before the current token,
        // suggest inserting the colon in between them, otherwise insert ": ".
        SourceLocation FILoc = Tok.getLocation();
        const char *FIText = ": ";
        const SourceManager &SM = PP.getSourceManager();
        if (FILoc.isFileID() /*|| PP.isAtStartOfMacroExpansion(FILoc, &FILoc)*/) {
          assert(FILoc.isFileID());
          bool IsInvalid = false;
          const char *SourcePtr =
              SM.getCharacterData(FILoc.getLocWithOffset(-1), &IsInvalid);
          if (!IsInvalid && *SourcePtr == ' ') {
            SourcePtr =
                SM.getCharacterData(FILoc.getLocWithOffset(-2), &IsInvalid);
            if (!IsInvalid && *SourcePtr == ' ') {
              FILoc = FILoc.getLocWithOffset(-1);
              FIText = ":";
            }
          }
        }

        // Diag(Tok, diag::err_expected)
        //     << tok::colon << FixItHint::CreateInsertion(FILoc, FIText);
        // Diag(OpToken, diag::note_matching) << tok::question;
        ColonLoc = Tok.getLocation();
      }
    }

    // PreferredType.enterBinary(Actions, Tok.getLocation(), LHS.get(),
    //                           OpToken.getKind());

    // Parse another leaf here for the RHS of the operator.
    // ParseCastExpression works here because all RHS expressions in C have it
    // as a prefix, at least. However, in C++, an assignment-expression could
    // be a throw-expression, which is not a valid cast-expression.
    // Therefore we need some special-casing here.
    // Also note that the third operand of the conditional operator is
    // an assignment-expression in C++, and in C++11, we can have a
    // braced-init-list on the RHS of an assignment. For better diagnostics,
    // parse as if we were allowed braced-init-lists everywhere, and check that
    // they only appear on the RHS of assignments later.
    ExprResult RHS;
    bool RHSIsInitList = false;
    if (/*getLangOpts().CPlusPlus11 &&*/ Tok.is(tok::l_brace)) {
      RHS = ParseBraceInitializer();
      RHSIsInitList = true;
    } else if (/*getLangOpts().CPlusPlus &&*/
               NextTokPrec <= prec::Conditional)
      RHS = ParseAssignmentExpression();
    else
      RHS = ParseCastExpression(AnyCastExpr);

    // if (RHS.isInvalid()) {
    //   // FIXME: Errors generated by the delayed typo correction should be
    //   // printed before errors from parsing the RHS, not after.
    //   Actions.CorrectDelayedTyposInExpr(LHS);
    //   if (TernaryMiddle.isUsable())
    //     TernaryMiddle = Actions.CorrectDelayedTyposInExpr(TernaryMiddle);
    //   LHS = ExprError();
    // }

    // Remember the precedence of this operator and get the precedence of the
    // operator immediately to the right of the RHS.
    prec::Level ThisPrec = NextTokPrec;
    NextTokPrec = getBinOpPrecedence(Tok.getKind(), GreaterThanIsOperator,
                                     /*getLangOpts().CPlusPlus11*/ true);

    // Assignment and conditional expressions are right-associative.
    bool isRightAssoc =
        ThisPrec == prec::Conditional || ThisPrec == prec::Assignment;

    // Get the precedence of the operator to the right of the RHS.  If it binds
    // more tightly with RHS than we do, evaluate it completely first.
    if (ThisPrec < NextTokPrec || (ThisPrec == NextTokPrec && isRightAssoc)) {
      if (!RHS.isInvalid() && RHSIsInitList) {
        // Diag(Tok, diag::err_init_list_bin_op)
        //     << /*LHS*/ 0 << PP.getSpelling(Tok)
        //     << Actions.getExprRange(RHS.get());

        RHS = ExprError();
      }

      //   // If this is left-associative, only parse things on the RHS that
      //   bind
      //   // more tightly than the current operator.  If it is
      //   left-associative, it
      //   // is okay, to bind exactly as tightly.  For example, compile A=B=C=D
      //   as
      //   // A=(B=(C=D)), where each paren is a level of recursion here.
      //   // The function takes ownership of the RHS.
      RHS = ParseRHSOfBinaryExpression(
          RHS, static_cast<prec::Level>(ThisPrec + !isRightAssoc));
      RHSIsInitList = false;

      // if (RHS.isInvalid()) {
      //   // FIXME: Errors generated by the delayed typo correction should be
      //   // printed before errors from ParseRHSOfBinaryExpression, not after.
      //   Actions.CorrectDelayedTyposInExpr(LHS);
      //   if (TernaryMiddle.isUsable())
      //     TernaryMiddle = Actions.CorrectDelayedTyposInExpr(TernaryMiddle);
      //   LHS = ExprError();
      // }

      NextTokPrec = getBinOpPrecedence(Tok.getKind(), GreaterThanIsOperator,
                                       /*getLangOpts().CPlusPlus11*/ true);
    }

    if (!RHS.isInvalid() && RHSIsInitList) {
      if (ThisPrec == prec::Assignment) {
        // Diag(OpToken, diag::warn_cxx98_compat_generalized_initializer_lists)
        //     << Actions.getExprRange(RHS.get());
      } else if (ColonLoc.isValid()) {
        // Diag(ColonLoc, diag::err_init_list_bin_op)
        //     << /*RHS*/ 1 << ":" << Actions.getExprRange(RHS.get());
        LHS = ExprError();
      } else {
        // Diag(OpToken, diag::err_init_list_bin_op)
        //     << /*RHS*/ 1 << PP.getSpelling(OpToken)
        //     << Actions.getExprRange(RHS.get());
        LHS = ExprError();
      }
    }

    ExprResult OrigLHS = LHS;
    if (!LHS.isInvalid()) {
      // Combine the LHS and RHS into the LHS (e.g. build AST).
      if (TernaryMiddle.isInvalid()) {
        // // If we're using '>>' as an operator within a template
        // // argument list (in C++98), suggest the addition of
        // // parentheses so that the code remains well-formed in C++0x.
        // if (!GreaterThanIsOperator && OpToken.is(tok::greatergreater))
        //   SuggestParentheses(OpToken.getLocation(),
        //                      diag::warn_cxx11_right_shift_in_template_arg,
        // SourceRange(Actions.getExprRange(LHS.get()).getBegin(),
        // Actions.getExprRange(RHS.get()).getEnd()));

        tok::TokenKind clangKind;
        ExprResult BinOp =
            Actions.ActOnBinOp(getCurScope(), OpToken.getLocation(), clangKind,
                               LHS.get(), RHS.get());

        if (BinOp.isInvalid())
          BinOp = Actions.CreateRecoveryExpr(LHS.get()->getBeginLoc(),
                                             RHS.get()->getBeginLoc(),
                                             {LHS.get(), RHS.get()});

        LHS = BinOp;
      } else {
        ExprResult CondOp = Actions.ActOnConditionalOp(
            OpToken.getLocation(), ColonLoc, LHS.get(), TernaryMiddle.get(),
            RHS.get());

        if (CondOp.isInvalid()) {
          std::vector<Expr *> Args;
          // TernaryMiddle can be null for the GNU conditional expr
          if (TernaryMiddle.get())
            Args = {LHS.get(), TernaryMiddle.get(), RHS.get()};
          else
            Args = {LHS.get(), RHS.get()};

          CondOp = Actions.CreateRecoveryExpr(LHS.get()->getBeginLoc(),
                                              RHS.get()->getEndLoc(), Args);
        }

        LHS = CondOp;
      }

      // In this case, ActOnBinOp or ActOnConditionalOp performed the
      // CorrectDelayedTyposInExpr check.
      if (/*!getLangOpts().CPlusPlus*/ true)
        continue;
    }

    // Ensure potential typos aren't left undiagnosed.
    // if (LHS.isInvalid()) {
    //   Actions.CorrectDelayedTyposInExpr(OrigLHS);
    //   Actions.CorrectDelayedTyposInExpr(TernaryMiddle);
    //   Actions.CorrectDelayedTyposInExpr(RHS);
    // }
  }
}