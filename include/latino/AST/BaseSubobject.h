//===- BaseSubobject.h - BaseSubobject class --------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides a definition of the BaseSubobject class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_AST_BASESUBOBJECT_H
#define LLVM_LATINO_AST_BASESUBOBJECT_H

#include "latino/AST/CharUnits.h"
#include "latino/AST/DeclCXX.h"
#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/Support/type_traits.h"
#include <cstdint>
#include <utility>

namespace latino {

class CXXRecordDecl;

// BaseSubobject - Uniquely identifies a direct or indirect base class.
// Stores both the base class decl and the offset from the most derived class to
// the base class. Used for vtable and VTT generation.
class BaseSubobject {
  /// Base - The base class declaration.
  const CXXRecordDecl *Base;

  /// BaseOffset - The offset from the most derived class to the base class.
  CharUnits BaseOffset;

public:
  BaseSubobject() = default;
  BaseSubobject(const CXXRecordDecl *Base, CharUnits BaseOffset)
      : Base(Base), BaseOffset(BaseOffset) {}

  /// getBase - Returns the base class declaration.
  const CXXRecordDecl *getBase() const { return Base; }

  /// getBaseOffset - Returns the base class offset.
  CharUnits getBaseOffset() const { return BaseOffset; }

  friend bool operator==(const BaseSubobject &LHS, const BaseSubobject &RHS) {
    return LHS.Base == RHS.Base && LHS.BaseOffset == RHS.BaseOffset;
 }
};

} // namespace latino

namespace llvm {

template<> struct DenseMapInfo<latino::BaseSubobject> {
  static latino::BaseSubobject getEmptyKey() {
    return latino::BaseSubobject(
      DenseMapInfo<const latino::CXXRecordDecl *>::getEmptyKey(),
      latino::CharUnits::fromQuantity(DenseMapInfo<int64_t>::getEmptyKey()));
  }

  static latino::BaseSubobject getTombstoneKey() {
    return latino::BaseSubobject(
      DenseMapInfo<const latino::CXXRecordDecl *>::getTombstoneKey(),
      latino::CharUnits::fromQuantity(DenseMapInfo<int64_t>::getTombstoneKey()));
  }

  static unsigned getHashValue(const latino::BaseSubobject &Base) {
    using PairTy = std::pair<const latino::CXXRecordDecl *, latino::CharUnits>;

    return DenseMapInfo<PairTy>::getHashValue(PairTy(Base.getBase(),
                                                     Base.getBaseOffset()));
  }

  static bool isEqual(const latino::BaseSubobject &LHS,
                      const latino::BaseSubobject &RHS) {
    return LHS == RHS;
  }
};

} // namespace llvm

#endif // LLVM_LATINO_AST_BASESUBOBJECT_H
