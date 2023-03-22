//===- BuildTree.h - build syntax trees -----------------------*- C++ -*-=====//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Functions to construct a syntax tree from an AST.
//===----------------------------------------------------------------------===//
#ifndef LLVM_LATINO_TOOLING_SYNTAX_TREE_H
#define LLVM_LATINO_TOOLING_SYNTAX_TREE_H

#include "latino/AST/Decl.h"
#include "latino/Basic/TokenKinds.h"
#include "latino/Tooling/Syntax/Nodes.h"
#include "latino/Tooling/Syntax/Tree.h"

namespace latino {
namespace syntax {

/// Build a syntax tree for the main file.
syntax::TranslationUnit *buildSyntaxTree(Arena &A,
                                         const latino::TranslationUnitDecl &TU);

// Create syntax trees from subtrees not backed by the source code.

latino::syntax::Leaf *createPunctuation(latino::syntax::Arena &A,
                                       latino::tok::TokenKind K);
latino::syntax::EmptyStatement *createEmptyStatement(latino::syntax::Arena &A);

} // namespace syntax
} // namespace latino
#endif
