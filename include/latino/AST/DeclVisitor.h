//===- DeclVisitor.h - Visitor for Decl subclasses --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the DeclVisitor interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_AST_DECLVISITOR_H
#define LLVM_LATINO_AST_DECLVISITOR_H

#include "latino/AST/Decl.h"
#include "latino/AST/DeclBase.h"
#include "latino/AST/DeclCXX.h"
#include "latino/AST/DeclFriend.h"
// #include "latino/AST/DeclObjC.h"
// #include "latino/AST/DeclOpenMP.h"
#include "latino/AST/DeclTemplate.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/ErrorHandling.h"

namespace latino {

namespace declvisitor {
/// A simple visitor class that helps create declaration visitors.
template <template <typename> class Ptr, typename ImplClass,
          typename RetTy = void>
class Base {
public:
#define PTR(CLASS) typename Ptr<CLASS>::type
#define DISPATCH(NAME, CLASS)                                                  \
  return static_cast<ImplClass *>(this)->Visit##NAME(static_cast<PTR(CLASS)>(D))

  RetTy Visit(PTR(Decl) D) {
    switch (D->getKind()) {
#define DECL(DERIVED, BASE)                                                    \
  case Decl::DERIVED:                                                          \
    DISPATCH(DERIVED##Decl, DERIVED##Decl);
#define ABSTRACT_DECL(DECL)
#include "latino/AST/DeclNodes.inc"
    }
    llvm_unreachable("Decl that isn't part of DeclNodes.inc!");
  }

  // If the implementation chooses not to implement a certain visit
  // method, fall back to the parent.
#define DECL(DERIVED, BASE)                                                    \
  RetTy Visit##DERIVED##Decl(PTR(DERIVED##Decl) D) { DISPATCH(BASE, BASE); }
#include "latino/AST/DeclNodes.inc"

  RetTy VisitDecl(PTR(Decl) D) { return RetTy(); }

#undef PTR
#undef DISPATCH
};

} // namespace declvisitor

/// A simple visitor class that helps create declaration visitors.
///
/// This class does not preserve constness of Decl pointers (see also
/// ConstDeclVisitor).
template <typename ImplClass, typename RetTy = void>
class DeclVisitor
    : public declvisitor::Base<std::add_pointer, ImplClass, RetTy> {};

/// A simple visitor class that helps create declaration visitors.
///
/// This class preserves constness of Decl pointers (see also DeclVisitor).
template <typename ImplClass, typename RetTy = void>
class ConstDeclVisitor
    : public declvisitor::Base<llvm::make_const_ptr, ImplClass, RetTy> {};

} // namespace latino

#endif // LLVM_LATINO_AST_DECLVISITOR_H
