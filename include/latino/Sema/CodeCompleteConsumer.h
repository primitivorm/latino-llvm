//===- CodeCompleteConsumer.h - Code Completion Interface -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the CodeCompleteConsumer class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_SEMA_CODECOMPLETECONSUMER_H
#define LLVM_LATINO_SEMA_CODECOMPLETECONSUMER_H

#include "CodeCompleteOptions.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/None.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/type_traits.h"
#include <cassert>
#include <memory>
#include <string>
#include <utility>

namespace latino {
/// Abstract interface for a consumer of code-completion
/// information.
class CodeCompleteConsumer {
protected:
  const CodeCompleteOptions CodeCompleteOpts;

public:
  CodeCompleteConsumer(const CodeCompleteOptions &CodeCompleteOpts)
      : CodeCompleteOpts(CodeCompleteOpts) {}
};
} // namespace latino

#endif