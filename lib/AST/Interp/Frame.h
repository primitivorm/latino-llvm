//===--- Frame.h - Call frame for the VM and AST Walker ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Defines the base class of interpreter and evaluator stack frames.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_AST_INTERP_FRAME_H
#define LLVM_LATINO_AST_INTERP_FRAME_H

#include "latino/Basic/SourceLocation.h"
#include "llvm/Support/raw_ostream.h"

namespace latino {
class FunctionDecl;

namespace interp {

/// Base class for stack frames, shared between VM and walker.
class Frame {
public:
  virtual ~Frame();

  /// Generates a human-readable description of the call site.
  virtual void describe(llvm::raw_ostream &OS) = 0;

  /// Returns a pointer to the caller frame.
  virtual Frame *getCaller() const = 0;

  /// Returns the location of the call site.
  virtual SourceLocation getCallLocation() const = 0;

  /// Returns the called function's declaration.
  virtual const FunctionDecl *getCallee() const = 0;
};

} // namespace interp
} // namespace latino

#endif
