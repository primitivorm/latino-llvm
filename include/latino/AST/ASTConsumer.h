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

namespace latino {
class ASTContext;
class Decl;
class DeclGroupRef;
class ASTContext;
class SemaConsumer; // layering violation required for safe SemaConsumer
class TagDecl;
class VarDecl;
class FunctionDecl;
class ImportDecl;

/// ASTConsumer - This is an abstract interface that should be implemented by
/// clients that read ASTs.  This abstraction layer allows the client to be
/// independent of the AST producer (e.g. parser vs AST dump file reader, etc).
class ASTConsumer {
  /// Whether this AST consumer also requires information about
  /// semantic analysis.
  bool SemaConsumer;

  friend class SemaConsumer;

public:
  ASTConsumer() {}
  virtual ~ASTConsumer() {}

  /// HandleTopLevelDecl - Handle the specified top-level declaration.  This is
  /// called by the parser to process every top-level Decl*.
  ///
  /// \returns true to continue parsing, or false to abort parsing.
  virtual bool HandleTopLevelDecl(DeclGroupRef D);

  /// HandleInterestingDecl - Handle the specified interesting declaration. This
  /// is called by the AST reader when deserializing things that might interest
  /// the consumer. The default implementation forwards to HandleTopLevelDecl.
  virtual void HandleInterestingDecl(DeclGroupRef D);

  /// HandleTranslationUnit - This method is called when the ASTs for entire
  /// translation unit have been parsed.
  virtual void HandleTranslationUnit(ASTContext &Ctx) {}

  /// Handle an ImportDecl that was implicitly created due to an
  /// inclusion directive.
  /// The default implementation passes it to HandleTopLevelDecl.
  // virtual void HandleImplicitImportDecl(ImportDecl *D);
};
} // namespace latino

#endif