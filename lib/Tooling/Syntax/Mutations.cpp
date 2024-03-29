//===- Mutations.cpp ------------------------------------------*- C++ -*-=====//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include "latino/Tooling/Syntax/Mutations.h"
#include "latino/Basic/LLVM.h"
#include "latino/Basic/SourceLocation.h"
#include "latino/Lex/Token.h"
#include "latino/Tooling/Core/Replacement.h"
#include "latino/Tooling/Syntax/BuildTree.h"
#include "latino/Tooling/Syntax/Nodes.h"
#include "latino/Tooling/Syntax/Tokens.h"
#include "latino/Tooling/Syntax/Tree.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/Casting.h"
#include <cassert>
#include <string>

using namespace latino;

// This class has access to the internals of tree nodes. Its sole purpose is to
// define helpers that allow implementing the high-level mutation operations.
class syntax::MutationsImpl {
public:
  /// Add a new node with a specified role.
  static void addAfter(syntax::Node *Anchor, syntax::Node *New, NodeRole Role) {
    assert(Anchor != nullptr);
    assert(New->Parent == nullptr);
    assert(New->NextSibling == nullptr);
    assert(!New->isDetached());
    assert(Role != NodeRole::Detached);

    New->setRole(Role);
    auto *P = Anchor->parent();
    P->replaceChildRangeLowLevel(Anchor, Anchor, New);

    P->assertInvariants();
  }

  /// Replace the node, keeping the role.
  static void replace(syntax::Node *Old, syntax::Node *New) {
    assert(Old != nullptr);
    assert(Old->Parent != nullptr);
    assert(Old->canModify());
    assert(New->Parent == nullptr);
    assert(New->NextSibling == nullptr);
    assert(New->isDetached());

    New->Role = Old->Role;
    auto *P = Old->parent();
    P->replaceChildRangeLowLevel(findPrevious(Old), Old->nextSibling(), New);

    P->assertInvariants();
  }

  /// Completely remove the node from its parent.
  static void remove(syntax::Node *N) {
    auto *P = N->parent();
    P->replaceChildRangeLowLevel(findPrevious(N), N->nextSibling(),
                                 /*New=*/nullptr);

    P->assertInvariants();
    N->assertInvariants();
  }

private:
  static syntax::Node *findPrevious(syntax::Node *N) {
    if (N->parent()->firstChild() == N)
      return nullptr;
    for (syntax::Node *C = N->parent()->firstChild(); C != nullptr;
         C = C->nextSibling()) {
      if (C->nextSibling() == N)
        return C;
    }
    llvm_unreachable("could not find a child node");
  }
};

void syntax::removeStatement(syntax::Arena &A, syntax::Statement *S) {
  assert(S);
  assert(S->canModify());

  if (isa<CompoundStatement>(S->parent())) {
    // A child of CompoundStatement can just be safely removed.
    MutationsImpl::remove(S);
    return;
  }
  // For the rest, we have to replace with an empty statement.
  if (isa<EmptyStatement>(S))
    return; // already an empty statement, nothing to do.

  MutationsImpl::replace(S, createEmptyStatement(A));
}
