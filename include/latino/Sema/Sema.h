//===--- Sema.h - Semantic Analysis & AST Building --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the Sema class, which performs semantic analysis and
// builds ASTs.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_SEMA_SEMA_H
#define LLVM_LATINO_SEMA_SEMA_H

#include "latino/AST/DeclBase.h"
#include "latino/Sema/CodeCompleteConsumer.h"
#include "latino/Sema/Scope.h"

namespace latino {
class ASTContext;
class ASTConsumer;
class Preprocessor;

class Sema final {
  Sema(const Sema &) = delete;
  void operator=(const Sema &) = delete;

public:
  typedef clang::OpaquePtr<clang::DeclGroupRef> DeclGroupPtrTy;
  Preprocessor &PP;
  ASTContext &Context;
  ASTConsumer &Consumer;

  /// Flag indicating whether or not to collect detailed statistics.
  bool CollectStats;

  Sema(Preprocessor &pp, ASTContext &ctx, ASTConsumer &consumer,
       TranslationUnitKind TUKind = TU_Complete,
       CodeCompleteConsumer *CompletionConsumer = nullptr);

  ~Sema();

  Preprocessor &getPreprocessor() const { return PP; }
  ASTConsumer &getASTConsumer() const { return Consumer; }

  DeclGroupPtrTy ConvertDeclToDeclGroup(Decl *Ptr, Decl *OwnedType = nullptr);

  /// Stmt attributes - this routine is the top level dispatcher.
  clang::StmtResult
  ProcessStmtAttributes(clang::Stmt *Stmt,
                        const clang::ParsedAttributesView &Attrs,
                        clang::SourceRange Range);

  clang::StmtResult ActOnExprStmtError();

  clang::StmtResult ActOnLabelStmt(clang::SourceLocation IdentLoc,
                                   clang::LabelDecl *TheDecl,
                                   clang::SourceLocation ColonLoc,
                                   clang::Stmt *subStmt);

  clang::ExprResult ActOnStmtExprResult(clang::ExprResult E);

  clang::StmtResult ActOnExprStmt(clang::ExprResult Arg,
                                  bool DiscardxCalie = true);

  void ActOnStartOfTranslationUnit();

  bool CheckCaseExpression(clang::Expr *E);

  clang::LabelDecl *LookupOrCreateLabel(
      IdentifierInfo *II, clang::SourceLocation IdentLoc,
      clang::SourceLocation GnuLabelLoc = clang::SourceLocation());

  clang::ExprResult ActOnBinOp(Scope *S, clang::SourceLocation TokLoc,
                               tok::TokenKind Kind, clang::Expr *LHSExpr,
                               clang::Expr *RHSExpr);

  /// Attempts to produce a RecoveryExpr after some AST node cannot be created.
  clang::ExprResult CreateRecoveryExpr(clang::SourceLocation Begin,
                                       clang::SourceLocation End,
                                       ArrayRef<clang::Expr *> SubExprs,
                                       clang::QualType T = clang::QualType());

  /// ActOnConditionalOp - Parse a ?: operation.  Note that 'LHS' may be null
  /// in the case of a the GNU conditional expr extension.
  clang::ExprResult ActOnConditionalOp(clang::SourceLocation QuestionLoc,
                                       clang::SourceLocation ColonLoc,
                                       clang::Expr *CondExpr,
                                       clang::Expr *LHSExpr,
                                       clang::Expr *RHSExpr);

  enum class ModuleDeclKind {
    Interface,      ///< 'export module X;'
    Implementation, ///< 'module X;'
  };

  /// The parser has processed a module-declaration that begins the definition
  /// of a module interface or implementation.
  DeclGroupPtrTy ActOnModuleDecl(clang::SourceLocation StartLoc,
                                 clang::SourceLocation ModuleLoc,
                                 ModuleDeclKind MDK, /*ModuleIdPath Path,*/
                                 bool IsFirstDecl);

private:
  /// The parser's current scope.
  ///
  /// The parser maintains this state here.
  Scope *CurScope;

  clang::StmtResult ActOnNullStmt(clang::SourceLocation SemiLoc,
                                  bool HasLeadingEmptyMacro = false);

protected:
  friend class Parser;

  /// Retrieve the parser's current scope.
  ///
  /// This routine must only be used when it is certain that semantic analysis
  /// and the parser are in precisely the same context, which is not the case
  /// when, e.g., we are performing any kind of template instantiation.
  /// Therefore, the only safe places to use this scope are in the parser
  /// itself and in routines directly invoked from the parser and *never* from
  /// template substitution or instantiation.
  Scope *getCurScope() const { return CurScope; }

  // public:
  //   ExprResult ActOnInitList(clang::SourceLocation LBraceLoc,
  //                            MultiExprArg InitArgList,
  //                            clang::SourceLocation RBraceLoc);
};
} // namespace latino

#endif
