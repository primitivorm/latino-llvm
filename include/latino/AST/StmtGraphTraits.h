//===- StmtGraphTraits.h - Graph Traits for the class Stmt ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines a template specialization of llvm::GraphTraits to
//  treat ASTs (Stmt*) as graphs
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_AST_STMTGRAPHTRAITS_H
#define LLVM_LATINO_AST_STMTGRAPHTRAITS_H

#include "latino/AST/Stmt.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"

namespace llvm {

template <> struct GraphTraits<latino::Stmt *> {
  using NodeRef = latino::Stmt *;
  using ChildIteratorType = latino::Stmt::child_iterator;
  using nodes_iterator = llvm::df_iterator<latino::Stmt *>;

  static NodeRef getEntryNode(latino::Stmt *S) { return S; }

  static ChildIteratorType child_begin(NodeRef N) {
    if (N) return N->child_begin();
    else return ChildIteratorType();
  }

  static ChildIteratorType child_end(NodeRef N) {
    if (N) return N->child_end();
    else return ChildIteratorType();
  }

  static nodes_iterator nodes_begin(latino::Stmt* S) {
    return df_begin(S);
  }

  static nodes_iterator nodes_end(latino::Stmt* S) {
    return df_end(S);
  }
};

template <> struct GraphTraits<const latino::Stmt *> {
  using NodeRef = const latino::Stmt *;
  using ChildIteratorType = latino::Stmt::const_child_iterator;
  using nodes_iterator = llvm::df_iterator<const latino::Stmt *>;

  static NodeRef getEntryNode(const latino::Stmt *S) { return S; }

  static ChildIteratorType child_begin(NodeRef N) {
    if (N) return N->child_begin();
    else return ChildIteratorType();
  }

  static ChildIteratorType child_end(NodeRef N) {
    if (N) return N->child_end();
    else return ChildIteratorType();
  }

  static nodes_iterator nodes_begin(const latino::Stmt* S) {
    return df_begin(S);
  }

  static nodes_iterator nodes_end(const latino::Stmt* S) {
    return df_end(S);
  }
};

} // namespace llvm

#endif // LLVM_LATINO_AST_STMTGRAPHTRAITS_H
