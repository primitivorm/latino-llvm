//=== MangleNumberingContext.h - Context for mangling numbers ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the LambdaBlockMangleContext interface, which keeps track
//  of the Itanium C++ ABI mangling numbers for lambda expressions and block
//  literals.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LATINO_AST_MANGLENUMBERINGCONTEXT_H
#define LLVM_LATINO_AST_MANGLENUMBERINGCONTEXT_H

#include "latino/Basic/LLVM.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"

namespace latino {

class BlockDecl;
class CXXMethodDecl;
class IdentifierInfo;
class TagDecl;
class Type;
class VarDecl;

/// Keeps track of the mangled names of lambda expressions and block
/// literals within a particular context.
class MangleNumberingContext {
public:
  virtual ~MangleNumberingContext() {}

  /// Retrieve the mangling number of a new lambda expression with the
  /// given call operator within this context.
  virtual unsigned getManglingNumber(const CXXMethodDecl *CallOperator) = 0;

  /// Retrieve the mangling number of a new block literal within this
  /// context.
  virtual unsigned getManglingNumber(const BlockDecl *BD) = 0;

  /// Static locals are numbered by source order.
  virtual unsigned getStaticLocalNumber(const VarDecl *VD) = 0;

  /// Retrieve the mangling number of a static local variable within
  /// this context.
  virtual unsigned getManglingNumber(const VarDecl *VD,
                                     unsigned MSLocalManglingNumber) = 0;

  /// Retrieve the mangling number of a static local variable within
  /// this context.
  virtual unsigned getManglingNumber(const TagDecl *TD,
                                     unsigned MSLocalManglingNumber) = 0;
};

} // end namespace latino
#endif
