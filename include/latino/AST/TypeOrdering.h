//===-------------- TypeOrdering.h - Total ordering for types ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Allows QualTypes to be sorted and hence used in maps and sets.
///
/// Defines latino::QualTypeOrdering, a total ordering on latino::QualType,
/// and hence enables QualType values to be sorted and to be used in
/// std::maps, std::sets, llvm::DenseMaps, and llvm::DenseSets.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_AST_TYPEORDERING_H
#define LLVM_LATINO_AST_TYPEORDERING_H

#include "latino/AST/CanonicalType.h"
#include "latino/AST/Type.h"
#include <functional>

namespace latino {

/// Function object that provides a total ordering on QualType values.
struct QualTypeOrdering {
  bool operator()(QualType T1, QualType T2) const {
    return std::less<void *>()(T1.getAsOpaquePtr(), T2.getAsOpaquePtr());
  }
};

} // namespace latino

namespace llvm {
template <class> struct DenseMapInfo;

template <> struct DenseMapInfo<latino::QualType> {
  static inline latino::QualType getEmptyKey() { return latino::QualType(); }

  static inline latino::QualType getTombstoneKey() {
    using latino::QualType;
    return QualType::getFromOpaquePtr(reinterpret_cast<latino::Type *>(-1));
  }

  static unsigned getHashValue(latino::QualType Val) {
    return (unsigned)((uintptr_t)Val.getAsOpaquePtr()) ^
           ((unsigned)((uintptr_t)Val.getAsOpaquePtr() >> 9));
  }

  static bool isEqual(latino::QualType LHS, latino::QualType RHS) {
    return LHS == RHS;
  }
};

template <> struct DenseMapInfo<latino::CanQualType> {
  static inline latino::CanQualType getEmptyKey() {
    return latino::CanQualType();
  }

  static inline latino::CanQualType getTombstoneKey() {
    using latino::CanQualType;
    return CanQualType::getFromOpaquePtr(reinterpret_cast<latino::Type *>(-1));
  }

  static unsigned getHashValue(latino::CanQualType Val) {
    return (unsigned)((uintptr_t)Val.getAsOpaquePtr()) ^
           ((unsigned)((uintptr_t)Val.getAsOpaquePtr() >> 9));
  }

  static bool isEqual(latino::CanQualType LHS, latino::CanQualType RHS) {
    return LHS == RHS;
  }
};
} // namespace llvm

#endif
