//===- Synthesis.cpp ------------------------------------------*- C++ -*-=====//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include "latino/Tooling/Syntax/BuildTree.h"

using namespace latino;

/// Exposes private syntax tree APIs required to implement node synthesis.
/// Should not be used for anything else.
class syntax::FactoryImpl {
public:
  static void setCanModify(syntax::Node *N) { N->CanModify = true; }

  static void prependChildLowLevel(syntax::Tree *T, syntax::Node *Child,
                                   syntax::NodeRole R) {
    T->prependChildLowLevel(Child, R);
  }
};

latino::syntax::Leaf *syntax::createPunctuation(latino::syntax::Arena &A,
                                               latino::tok::TokenKind K) {
  auto Tokens = A.lexBuffer(llvm::MemoryBuffer::getMemBuffer(
                                latino::tok::getPunctuatorSpelling(K)))
                    .second;
  assert(Tokens.size() == 1);
  assert(Tokens.front().kind() == K);
  auto *L = new (A.allocator()) latino::syntax::Leaf(Tokens.begin());
  FactoryImpl::setCanModify(L);
  L->assertInvariants();
  return L;
}

latino::syntax::EmptyStatement *
syntax::createEmptyStatement(latino::syntax::Arena &A) {
  auto *S = new (A.allocator()) latino::syntax::EmptyStatement;
  FactoryImpl::setCanModify(S);
  FactoryImpl::prependChildLowLevel(S, createPunctuation(A, latino::tok::semi),
                                    NodeRole::Unknown);
  S->assertInvariants();
  return S;
}
