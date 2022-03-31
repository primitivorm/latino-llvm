//===- DeclBase.h - Base Classes for representing declarations --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the Decl and DeclContext interfaces.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_AST_DECLBASE_H
#define LLVM_LATINO_AST_DECLBASE_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/PointerIntPair.h"
#include "llvm/ADT/PointerUnion.h"
#include "llvm/ADT/iterator.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/VersionTuple.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <string>
#include <type_traits>
#include <utility>

namespace latino {

/// Decl - This represents one declaration (or definition), e.g. a variable,
/// typedef, function, struct, etc.
///
/// Note: There are objects tacked on before the *beginning* of Decl
/// (and its subclasses) in its Decl::operator new(). Proper alignment
/// of all subclasses (not requiring more than the alignment of Decl) is
/// asserted in DeclBase.cpp.
class alignas(8) Decl {

public:
  Decl() = delete;
  Decl(const Decl &) = delete;
  Decl(Decl &&) = delete;
  Decl &operator=(const Decl &) = delete;
  Decl &operator=(Decl &&) = delete;

  // global temp stats (until we have a per-module visitor)
  static void EnableStatistics();
};

} // namespace latino

#endif