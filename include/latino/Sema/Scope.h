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

#include "latino/AST/Decl.h"
#include "clang/Basic/Diagnostic.h"
#include "llvm/ADT/PointerIntPair.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/iterator_range.h"
#include <cassert>

namespace llvm {

class raw_ostream;

} // namespace llvm

namespace latino {

class Decl;
class DeclContext;

/// Scope - A scope is a transient data structure that is used while parsing the
/// program.  It assists with resolving identifiers to the appropriate
/// declaration.
class Scope {

private:
  /// Used to determine if errors occurred in this scope.
  clang::DiagnosticErrorTrap ErrorTrap;

  /// Declarations with static linkage are mangled with the number of
  /// scopes seen as a component.
  unsigned short MSLastManglingNumber;
  unsigned short MSCurManglingNumber;

  Scope *MSLastManglingParent;

public:
  Scope(Scope *Parent, unsigned ScopeFlags, clang::DiagnosticsEngine &Diag)
      : ErrorTrap(Diag) {
    Init(Parent, ScopeFlags);
  }

  /// Init - This is used by the parser to implement scope caching.
  void Init(Scope *parent, unsigned flags);

  const Scope *getMSLastManglingParent() const { return MSLastManglingParent; }
  Scope *getMSLastManglingParent() { return MSLastManglingParent; }

  void incrementMSManglingNumber() {
    if (Scope *MSLMP = getMSLastManglingParent()) {
      MSLMP->MSLastManglingNumber += 1;
      MSCurManglingNumber += 1;
    }
  }
};

} // namespace latino

#endif