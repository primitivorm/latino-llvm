//===--- ASTDiagnostic.h - Diagnostics for the AST library ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_AST_ASTDIAGNOSTIC_H
#define LLVM_LATINO_AST_ASTDIAGNOSTIC_H

#include "latino/Basic/Diagnostic.h"
#include "latino/Basic/DiagnosticAST.h"

namespace latino {
  /// DiagnosticsEngine argument formatting function for diagnostics that
  /// involve AST nodes.
  ///
  /// This function formats diagnostic arguments for various AST nodes,
  /// including types, declaration names, nested name specifiers, and
  /// declaration contexts, into strings that can be printed as part of
  /// diagnostics. It is meant to be used as the argument to
  /// \c DiagnosticsEngine::SetArgToStringFn(), where the cookie is an \c
  /// ASTContext pointer.
  void FormatASTNodeDiagnosticArgument(
      DiagnosticsEngine::ArgumentKind Kind,
      intptr_t Val,
      StringRef Modifier,
      StringRef Argument,
      ArrayRef<DiagnosticsEngine::ArgumentValue> PrevArgs,
      SmallVectorImpl<char> &Output,
      void *Cookie,
      ArrayRef<intptr_t> QualTypeVals);
}  // end namespace latino

#endif
