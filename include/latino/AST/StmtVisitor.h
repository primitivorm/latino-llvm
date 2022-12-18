//===- StmtVisitor.h - Visitor for Stmt subclasses --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the StmtVisitor and ConstStmtVisitor interfaces.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_AST_STMTVISITOR_H
#define LLVM_LATINO_AST_STMTVISITOR_H

#include "latino/AST/Stmt.h"
#include "latino/Basic/LLVM.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include <utility>

namespace latino {}

#endif