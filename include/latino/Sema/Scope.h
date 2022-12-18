//===- Scope.h - Scope interface --------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the Scope interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_SEMA_SCOPE_H
#define LLVM_LATINO_SEMA_SCOPE_H

#include "latino/Basic/Diagnostic.h"

namespace latino {
class Scope {
public:
  /// ScopeFlags - These are bitfields that are or'd together when creating a
  /// scope, which defines the sorts of things the scope contains.
  enum ScopeFlags {
    /// This indicates that the scope corresponds to a function, which
    /// means that labels are set here.
    FnScope = 0x01,

    /// This is a while, do, switch, for, etc that can have break
    /// statements embedded into it.
    BreakScope = 0x02,

    /// This is a while, do, for, which can have continue statements
    /// embedded into it.
    ContinueScope = 0x04,

    /// This is a scope that can contain a declaration.  Some scopes
    /// just contain loop constructs but don't contain decls.
    DeclScope = 0x08,

    /// The controlling scope in a if/switch/while/for statement.
    ControlScope = 0x10,

    /// The scope of a struct/union/class definition.
    ClassScope = 0x20,

    /// This is a scope that corresponds to a block/closure object.
    /// Blocks serve as top-level scopes for some objects like labels, they
    /// also prevent things like break and continue.  BlockScopes always have
    /// the FnScope and DeclScope flags set as well.
    BlockScope = 0x40,

    /// This is a scope that corresponds to the
    /// parameters within a function prototype.
    FunctionPrototypeScope = 0x100,

    /// This is a scope that corresponds to the parameters within
    /// a function prototype for a function declaration (as opposed to any
    /// other kind of function declarator). Always has FunctionPrototypeScope
    /// set as well.
    FunctionDeclarationScope = 0x200,

    /// This is a scope that corresponds to the Objective-C
    /// \@catch statement.
    AtCatchScope = 0x400,

    /// This scope corresponds to an Objective-C method body.
    /// It always has FnScope and DeclScope set as well.
    // ObjCMethodScope = 0x800,

    /// This is a scope that corresponds to a switch statement.
    SwitchScope = 0x1000,

    /// This is the scope of a C++ try statement.
    TryScope = 0x2000,

    /// This is the scope for a function-level C++ try or catch scope.
    FnTryCatchScope = 0x4000,

    /// This is the scope of OpenMP executable directive.
    // OpenMPDirectiveScope = 0x8000,

    /// This is the scope of some OpenMP loop directive.
    // OpenMPLoopDirectiveScope = 0x10000,

    /// This is the scope of some OpenMP simd directive.
    /// For example, it is used for 'omp simd', 'omp for simd'.
    /// This flag is propagated to children scopes.
    // OpenMPSimdDirectiveScope = 0x20000,

    /// This scope corresponds to an enum.
    EnumScope = 0x40000,

    /// This scope corresponds to an SEH try.
    SEHTryScope = 0x80000,

    /// This scope corresponds to an SEH except.
    SEHExceptScope = 0x100000,

    /// We are currently in the filter expression of an SEH except block.
    SEHFilterScope = 0x200000,

    /// This is a compound statement scope.
    CompoundStmtScope = 0x400000,

    /// We are between inheritance colon and the real class/struct definition
    /// scope.
    ClassInheritanceScope = 0x800000,

    /// This is the scope of a C++ catch statement.
    CatchScope = 0x1000000,

  };

private:
  /// The parent scope for this scope.  This is null for the translation-unit
  /// scope.
  Scope *AnyParent;

  /// Flags - This contains a set of ScopeFlags, which indicates how the scope
  /// interrelates with other control flow statements.
  unsigned Flags;

  /// Used to determine if errors occurred in this scope.
  DiagnosticErrorTrap ErrorTrap;

public:
  Scope(Scope *Parent, unsigned ScopeFlags, DiagnosticsEngine &Diag)
      : ErrorTrap(Diag) {
    Init(Parent, ScopeFlags);
  }

  /// getFlags - Return the flags for this scope.
  unsigned getFlags() const { return Flags; }

  /// getParent - Return the scope that this is nested in.
  const Scope *getParent() const { return AnyParent; }

  /// isSwitchScope - Return true if this scope is a switch scope.
  bool isSwitchScope() const {
    for (const Scope *S = this; S; S->getParent()) {
      if (S->getFlags() & Scope::SwitchScope)
        return true;
      else if (S->getFlags() &
               (Scope::FnScope | Scope::ClassScope | Scope::BlockScope |
                Scope::FunctionPrototypeScope | Scope::AtCatchScope))
        return false;
    }
    return false;
  }

  /// Init - This is used by the parser to implement scope caching.
  void Init(Scope *parent, unsigned flags);
};
} // namespace latino
#endif
