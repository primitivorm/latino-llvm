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

#include "latino/AST/ExternalASTSource.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/FoldingSet.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/None.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/PointerIntPair.h"
#include "llvm/ADT/PointerUnion.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/TinyPtrVector.h"
#include "llvm/ADT/Triple.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/Support/AlignOf.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/TypeSize.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace llvm {

struct fltSemantics;
template <typename T, unsigned N> class SmallPtrSet;

} // namespace llvm

namespace latino {

    /// Holds long-lived AST nodes (such as types and decls) that can be
    /// referred to throughout the semantic analysis of a file.
  class ASTContext: public llvm::RefCountedBase<ASTContext> {

    public:
    llvm::IntrusiveRefCntPtr<ExternalASTSource> ExternalSource;
    

    /// Retrieve a pointer to the external AST source associated
  /// with this AST context, if any.
  ExternalASTSource *getExternalSource() const {
    return ExternalSource.get();
  }

  // void PrintStats() const;

  };
}

#endif