//===--- IndexingOptions.h - Options for indexing ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_INDEX_INDEXINGOPTIONS_H
#define LLVM_LATINO_INDEX_INDEXINGOPTIONS_H

#include "latino/Frontend/FrontendOptions.h"
#include <memory>
#include <string>

namespace latino {
class Decl;
namespace index {

struct IndexingOptions {
  enum class SystemSymbolFilterKind {
    None,
    DeclarationsOnly,
    All,
  };

  SystemSymbolFilterKind SystemSymbolFilter =
      SystemSymbolFilterKind::DeclarationsOnly;
  bool IndexFunctionLocals = false;
  bool IndexImplicitInstantiation = false;
  // Whether to index macro definitions in the Preprocesor when preprocessor
  // callback is not available (e.g. after parsing has finished). Note that
  // macro references are not available in Proprocessor.
  bool IndexMacrosInPreprocessor = false;
  // Has no effect if IndexFunctionLocals are false.
  bool IndexParametersInDeclarations = false;
  bool IndexTemplateParameters = false;

  // If set, skip indexing inside some declarations for performance.
  // This prevents traversal, so skipping a struct means its declaration an
  // members won't be indexed, but references elsewhere to that struct will be.
  // Currently this is only checked for top-level declarations.
  std::function<bool(const Decl *)> ShouldTraverseDecl;
};

} // namespace index
} // namespace latino

#endif // LLVM_LATINO_INDEX_INDEXINGOPTIONS_H
