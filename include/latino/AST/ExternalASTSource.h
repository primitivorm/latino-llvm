//===- ExternalASTSource.h - Abstract External AST Interface ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the ExternalASTSource interface, which enables
//  construction of AST nodes from some external source.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_AST_EXTERNALASTSOURCE_H
#define LLVM_LATINO_AST_EXTERNALASTSOURCE_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h""
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/PointerUnion.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/iterator.h"
#include "llvm/Support/PointerLikeTypeTraits.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <string>
#include <utility>

namespace llvm {

struct fltSemantics;
template <typename T, unsigned N> class SmallPtrSet;

} // namespace llvm

namespace latino {

/// Abstract interface for external sources of AST nodes.
///
/// External AST sources provide AST nodes constructed from some
/// external source, such as a precompiled header. External AST
/// sources can resolve types and declarations from abstract IDs into
/// actual type and declaration nodes, and read parts of declaration
/// contexts.
class ExternalASTSource : public llvm::RefCountedBase<ExternalASTSource> {
public:
  ExternalASTSource() = default;
  virtual ~ExternalASTSource();
};
} // namespace latino

#endif