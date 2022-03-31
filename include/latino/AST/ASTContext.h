//===- ASTContext.h - Context to hold long-lived AST nodes ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Defines the latino::ASTContext interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_AST_ASTCONTEXT_H
#define LLVM_LATINO_AST_ASTCONTEXT_H

#include "latino/AST/Decl.h"
#include "latino/AST/DeclBase.h"

namespace latino {

/// Holds long-lived AST nodes (such as types and decls) that can be
/// referred to throughout the semantic analysis of a file.
class ASTContext : public RefCountedBase<ASTContext> {};

} // namespace latino

#endif // LLVM_LATINO_AST_ASTCONTEXT_