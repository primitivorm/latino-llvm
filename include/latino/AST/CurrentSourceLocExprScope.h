//===--- CurrentSourceLocExprScope.h ----------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines types used to track the current context needed to evaluate
//  a SourceLocExpr.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_AST_CURRENT_SOURCE_LOC_EXPR_SCOPE_H
#define LLVM_LATINO_AST_CURRENT_SOURCE_LOC_EXPR_SCOPE_H

#include <cassert>

namespace latino {
class Expr;

/// Represents the current source location and context used to determine the
/// value of the source location builtins (ex. __builtin_LINE), including the
/// context of default argument and default initializer expressions.
class CurrentSourceLocExprScope {
  /// The CXXDefaultArgExpr or CXXDefaultInitExpr we're currently evaluating.
  const Expr *DefaultExpr = nullptr;

public:
  /// A RAII style scope guard used for tracking the current source
  /// location and context as used by the source location builtins
  /// (ex. __builtin_LINE).
  class SourceLocExprScopeGuard;

  const Expr *getDefaultExpr() const { return DefaultExpr; }

  explicit CurrentSourceLocExprScope() = default;

private:
  explicit CurrentSourceLocExprScope(const Expr *DefaultExpr)
      : DefaultExpr(DefaultExpr) {}

  CurrentSourceLocExprScope(CurrentSourceLocExprScope const &) = default;
  CurrentSourceLocExprScope &
  operator=(CurrentSourceLocExprScope const &) = default;
};

class CurrentSourceLocExprScope::SourceLocExprScopeGuard {
public:
  SourceLocExprScopeGuard(const Expr *DefaultExpr,
                          CurrentSourceLocExprScope &Current)
      : Current(Current), OldVal(Current), Enable(false) {
    assert(DefaultExpr && "the new scope should not be empty");
    if ((Enable = (Current.getDefaultExpr() == nullptr)))
      Current = CurrentSourceLocExprScope(DefaultExpr);
  }

  ~SourceLocExprScopeGuard() {
    if (Enable)
      Current = OldVal;
  }

private:
  SourceLocExprScopeGuard(SourceLocExprScopeGuard const &) = delete;
  SourceLocExprScopeGuard &operator=(SourceLocExprScopeGuard const &) = delete;

  CurrentSourceLocExprScope &Current;
  CurrentSourceLocExprScope OldVal;
  bool Enable;
};

} // end namespace latino

#endif // LLVM_LATINO_AST_CURRENT_SOURCE_LOC_EXPR_SCOPE_H
