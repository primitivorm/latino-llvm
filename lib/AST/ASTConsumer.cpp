//===--- ASTConsumer.cpp - Abstract interface for reading ASTs --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the ASTConsumer class.
//
//===----------------------------------------------------------------------===//

#include "latino/AST/ASTConsumer.h"
#include "latino/AST/Decl.h"
#include "latino/AST/DeclGroup.h"
using namespace latino;

bool ASTConsumer::HandleTopLevelDecl(DeclGroupRef D) {
  return true;
}

void ASTConsumer::HandleInterestingDecl(DeclGroupRef D) {
  HandleTopLevelDecl(D);
}

void ASTConsumer::HandleTopLevelDeclInObjCContainer(DeclGroupRef D) {}

void ASTConsumer::HandleImplicitImportDecl(ImportDecl *D) {
  HandleTopLevelDecl(DeclGroupRef(D));
}
