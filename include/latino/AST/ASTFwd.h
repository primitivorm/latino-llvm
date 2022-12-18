//===--- ASTFwd.h ----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===--------------------------------------------------------------===//
///
/// \file
/// Forward declaration of all AST node types.
///
//===-------------------------------------------------------------===//

#ifndef LLVM_LATINO_AST_ASTFWD_H
#define LLVM_LATINO_AST_ASTFWD_H

namespace latino {

class Decl;
#define DECL(DERIVED, BASE) class DERIVED##Decl;
#include "latino/AST/DeclNodes.inc"
class Stmt;
#define STMT(DERIVED, BASE) class DERIVED;
#include "latino/AST/StmtNodes.inc"
class Type;
#define TYPE(DERIVED, BASE) class DERIVED##Type;
#include "latino/AST/TypeNodes.inc"

} // namespace latino
#endif