//===- Stmt.cpp - Statement AST Node Implementation -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the Stmt class and statement subclasses.
//
//===----------------------------------------------------------------------===//

#include "latino/AST/Stmt.h"
#include "latino/AST/Expr.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <string>
#include <type_traits>
#include <utility>

using namespace latino;

static StmtClassNameTable &getStmtInfoTableEntry(Stmt::StmtClass E) {
  static bool Initialized = false;
  if (Initialized)
    return StmtClassInfo[E];

  // Initialize the table on the first use.
  Initialized = true;
#define ABSTRACT_STMT(STMT)
#define STMT(CLASS, PARENT)                                                    \
  StmtClassInfo[(unsigned)Stmt::CLASS##Class].Name = #CLASS;                   \
  StmtClassInfo[(unsigned)Stmt::CLASS##Class].Size = sizeof(CLASS);
#include "latino/AST/StmtNodes.inc"

  return StmtClassInfo[E];
}

bool Stmt::StatisticsEnabled = false;
void Stmt::EnableStatistics() { StatisticsEnabled = true; }

void Stmt::addStmtClass(StmtClass s) { ++getStmtInfoTableEntry(s).Counter; }

const Expr *ValueStmt::getExprStmt() const {
  const Stmt *S = this;
  do {
    if (const auto *E = dyn_cast<Expr>(S))
      return E;

    llvm_unreachable("unknown kind of ValueStmt");
  } while (isa<ValueStmt>(S));

  return nullptr;
}