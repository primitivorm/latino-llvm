//===--- Specifiers.h - Declaration and Type Specifiers ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Defines various enumerations that describe declaration and
/// type specifiers.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_BASIC_SPECIFIERS_H
#define LLVM_LATINO_BASIC_SPECIFIERS_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/ErrorHandling.h"

namespace latino {
    /// Describes the nullability of a particular type.
  enum class NullabilityKind : uint8_t {
    /// Values of this type can never be null.
    NonNull = 0,
    /// Values of this type can be null.
    Nullable,
    /// Whether values of this type can be null is (explicitly)
    /// unspecified. This captures a (fairly rare) case where we
    /// can't conclude anything about the nullability of the type even
    /// though it has been considered.
    Unspecified
  };
}

#endif