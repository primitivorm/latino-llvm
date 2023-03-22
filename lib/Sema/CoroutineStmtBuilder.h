//===- CoroutineStmtBuilder.h - Implicit coroutine stmt builder -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//===----------------------------------------------------------------------===//
//
//  This file defines CoroutineStmtBuilder, a class for building the implicit
//  statements required for building a coroutine body.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_LIB_SEMA_COROUTINESTMTBUILDER_H
#define LLVM_LATINO_LIB_SEMA_COROUTINESTMTBUILDER_H

#include "latino/AST/Decl.h"
#include "latino/AST/ExprCXX.h"
#include "latino/AST/StmtCXX.h"
#include "latino/Lex/Preprocessor.h"
#include "latino/Sema/SemaInternal.h"

namespace latino {

class CoroutineStmtBuilder : public CoroutineBodyStmt::CtorArgs {
  Sema &S;
  FunctionDecl &FD;
  sema::FunctionScopeInfo &Fn;
  bool IsValid = true;
  SourceLocation Loc;
  SmallVector<Stmt *, 4> ParamMovesVector;
  const bool IsPromiseDependentType;
  CXXRecordDecl *PromiseRecordDecl = nullptr;

public:
  /// Construct a CoroutineStmtBuilder and initialize the promise
  /// statement and initial/final suspends from the FunctionScopeInfo.
  CoroutineStmtBuilder(Sema &S, FunctionDecl &FD, sema::FunctionScopeInfo &Fn,
                       Stmt *Body);

  /// Build the coroutine body statements, including the
  /// "promise dependent" statements when the promise type is not dependent.
  bool buildStatements();

  /// Build the coroutine body statements that require a non-dependent
  /// promise type in order to construct.
  ///
  /// For example different new/delete overloads are selected depending on
  /// if the promise type provides `unhandled_exception()`, and therefore they
  /// cannot be built until the promise type is complete so that we can perform
  /// name lookup.
  bool buildDependentStatements();

  bool isInvalid() const { return !this->IsValid; }

private:
  bool makePromiseStmt();
  bool makeInitialAndFinalSuspend();
  bool makeNewAndDeleteExpr();
  bool makeOnFallthrough();
  bool makeOnException();
  bool makeReturnObject();
  bool makeGroDeclAndReturnStmt();
  bool makeReturnOnAllocFailure();
};

} // end namespace latino

#endif // LLVM_LATINO_LIB_SEMA_COROUTINESTMTBUILDER_H
