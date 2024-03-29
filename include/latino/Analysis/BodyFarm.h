//== BodyFarm.h - Factory for conjuring up fake bodies -------------*- C++ -*-//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// BodyFarm is a factory for creating faux implementations for functions/methods
// for analysis purposes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_LIB_ANALYSIS_BODYFARM_H
#define LLVM_LATINO_LIB_ANALYSIS_BODYFARM_H

#include "latino/AST/DeclBase.h"
#include "latino/Basic/LLVM.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Optional.h"

namespace latino {

class ASTContext;
class FunctionDecl;
// class ObjCMethodDecl;
// class ObjCPropertyDecl;
class Stmt;
class CodeInjector;

class BodyFarm {
public:
  BodyFarm(ASTContext &C, CodeInjector *injector) : C(C), Injector(injector) {}

  /// Factory method for creating bodies for ordinary functions.
  Stmt *getBody(const FunctionDecl *D);

  /// Factory method for creating bodies for Objective-C properties.
  // Stmt *getBody(const ObjCMethodDecl *D);

  /// Remove copy constructor to avoid accidental copying.
  BodyFarm(const BodyFarm &other) = delete;

private:
  typedef llvm::DenseMap<const Decl *, Optional<Stmt *>> BodyMap;

  ASTContext &C;
  BodyMap Bodies;
  CodeInjector *Injector;
};
} // namespace latino

#endif
