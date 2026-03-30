//===--- ASTConsumer.h - Abstract interface for reading ASTs ----*- C++ -*-===//
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

#ifndef LLVM_LATINO_AST_ASTCONSUMER_H
#define LLVM_LATINO_AST_ASTCONSUMER_H

#include "latino/AST/ASTContext.h"
#include "latino/AST/DeclGroup.h"

namespace latino {
/// ASTConsumer - This is an abstract interface that should be implemented by
/// clients that read ASTs.  This abstraction layer allows the client to be
/// independent of the AST producer (e.g. parser vs AST dump file reader, etc).
class ASTConsumer {
  /// Whether this AST consumer also requires information about
  /// semantic analysis.
  bool SemaConsumer;

  friend class SemaConsumer;
public:
  ASTConsumer() : SemaConsumer(false) { }

  virtual ~ASTConsumer() {}

  /// Initialize - This is called to initialize the consumer, providing the
  /// ASTContext.
  virtual void Initialize(ASTContext &Context) {}

  /// HandleTopLevelDecl - Handle the specified top-level declaration.  This is
  /// called by the parser to process every top-level Decl*.
  ///
  /// \returns true to continue parsing, or false to abort parsing.
  virtual bool HandleTopLevelDecl(DeclGroupRef D);

  /// HandleTranslationUnit - This method is called when the ASTs for entire
  /// translation unit have been parsed.
  virtual void HandleTranslationUnit(ASTContext &Ctx) {}
};

}

#endif