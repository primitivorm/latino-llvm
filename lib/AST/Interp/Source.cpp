//===--- Source.cpp - Source expression tracking ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Source.h"
#include "latino/AST/Expr.h"

using namespace latino;
using namespace latino::interp;

SourceLocation SourceInfo::getLoc() const {
  if (const Expr *E = asExpr())
    return E->getExprLoc();
  if (const Stmt *S = asStmt())
    return S->getBeginLoc();
  if (const Decl *D = asDecl())
    return D->getBeginLoc();
  return SourceLocation();
}

const Expr *SourceInfo::asExpr() const {
  if (auto *S = Source.dyn_cast<const Stmt *>())
    return dyn_cast<Expr>(S);
  return nullptr;
}

const Expr *SourceMapper::getExpr(Function *F, CodePtr PC) const {
  if (const Expr *E = getSource(F, PC).asExpr())
    return E;
  llvm::report_fatal_error("missing source expression");
}

SourceLocation SourceMapper::getLocation(Function *F, CodePtr PC) const {
  return getSource(F, PC).getLoc();
}
