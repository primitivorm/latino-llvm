//===- DeclObjC.h - Classes for representing declarations -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the DeclObjC interface and subclasses.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_DECLOBJC_H
#define LLVM_CLANG_AST_DECLOBJC_H

#include "latino/AST/Decl.h"
#include "latino/AST/DeclBase.h"
// #include "latino/AST/DeclObjCCommon.h"
#include "latino/AST/ExternalASTSource.h"
#include "latino/AST/Redeclarable.h"
// #include "latino/AST/SelectorLocationsKind.h"
#include "latino/AST/Type.h"
#include "latino/Basic/IdentifierTable.h"
#include "latino/Basic/LLVM.h"
#include "latino/Basic/SourceLocation.h"
#include "latino/Basic/Specifiers.h"

namespace latino {
    class ObjCIvarDecl;


    /// ObjCIvarDecl - Represents an ObjC instance variable. In general, ObjC
/// instance variables are identical to C. The only exception is Objective-C
/// supports C++ style access control. For example:
///
///   \@interface IvarExample : NSObject
///   {
///     id defaultToProtected;
///   \@public:
///     id canBePublic; // same as C++.
///   \@protected:
///     id canBeProtected; // same as C++.
///   \@package:
///     id canBePackage; // framework visibility (not available in C++).
///   }
///
class ObjCIvarDecl : public FieldDecl {
  void anchor() override;

public:
  enum AccessControl {
    None, Private, Protected, Public, Package
  };

private:
  ObjCIvarDecl(/*ObjCContainerDecl*/ DeclContext *DC, SourceLocation StartLoc,
               SourceLocation IdLoc, IdentifierInfo *Id,
               QualType T, TypeSourceInfo *TInfo, AccessControl ac, Expr *BW,
               bool synthesized)
      : FieldDecl(ObjCIvar, DC, StartLoc, IdLoc, Id, T, TInfo, BW,
                  /*Mutable=*/false, /*HasInit=*/ICIS_NoInit),
        DeclAccess(ac), Synthesized(synthesized) {}

public:
//   static ObjCIvarDecl *Create(ASTContext &C, /*ObjCContainerDecl *DC,*/
//                               SourceLocation StartLoc, SourceLocation IdLoc,
//                               IdentifierInfo *Id, QualType T,
//                               TypeSourceInfo *TInfo,
//                               AccessControl ac, Expr *BW = nullptr,
//                               bool synthesized=false);

//   static ObjCIvarDecl *CreateDeserialized(ASTContext &C, unsigned ID);

  /// Return the class interface that this ivar is logically contained
  /// in; this is either the interface where the ivar was declared, or the
  /// interface the ivar is conceptually a part of in the case of synthesized
  /// ivars.
//   const ObjCInterfaceDecl *getContainingInterface() const;

  ObjCIvarDecl *getNextIvar() { return NextIvar; }
  const ObjCIvarDecl *getNextIvar() const { return NextIvar; }
  void setNextIvar(ObjCIvarDecl *ivar) { NextIvar = ivar; }

  void setAccessControl(AccessControl ac) { DeclAccess = ac; }

  AccessControl getAccessControl() const { return AccessControl(DeclAccess); }

  AccessControl getCanonicalAccessControl() const {
    return DeclAccess == None ? Protected : AccessControl(DeclAccess);
  }

  void setSynthesize(bool synth) { Synthesized = synth; }
  bool getSynthesize() const { return Synthesized; }

  /// Retrieve the type of this instance variable when viewed as a member of a
  /// specific object type.
//   QualType getUsageType(QualType objectType) const;

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == ObjCIvar; }

private:
  /// NextIvar - Next Ivar in the list of ivars declared in class; class's
  /// extensions and class's implementation
  ObjCIvarDecl *NextIvar = nullptr;

  // NOTE: VC++ treats enums as signed, avoid using the AccessControl enum
  unsigned DeclAccess : 3;
  unsigned Synthesized : 1;
};

}

#endif