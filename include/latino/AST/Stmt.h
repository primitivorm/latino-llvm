//===- Stmt.h - Classes for representing statements -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the Stmt interface and subclasses.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_AST_STMT_H
#define LLVM_LATINO_AST_STMT_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/BitmaskEnum.h"
#include "llvm/ADT/PointerIntPair.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/iterator.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/ErrorHandling.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <string>

namespace latino {

//===----------------------------------------------------------------------===//
// AST classes for statements.
//===----------------------------------------------------------------------===//

/// Stmt - This represents one statement.
///
class alignas(void *) Stmt {

public:
  Stmt() = delete;
  Stmt(const Stmt &) = delete;
  Stmt(Stmt &) = delete;
  Stmt &operator=(const Stmt &) = delete;
  Stmt &operator=(Stmt &) = delete;

  // global temp stats (until we have a per-module visitor)
  static void EnableStatistics();
};

} // namespace latino

#endif