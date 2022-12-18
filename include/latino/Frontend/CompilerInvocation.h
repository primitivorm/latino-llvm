//===- CompilerInvocation.h - Compiler Invocation Helper Data ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_FRONTEND_COMPILERINVOCATION_H
#define LLVM_LATINO_FRONTEND_COMPILERINVOCATION_H

#include "clang/Lex/PreprocessorOptions.h"

#include "latino/Basic/LangOptions.h"
#include "latino/Basic/TargetOptions.h"
#include "latino/Frontend/FrontendOptions.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include <memory>
#include <string>

namespace latino {
class clang::PreprocessorOptions;

class CompilerInvocationBase {
public:
  /// Options controlling the target.
  std::shared_ptr<TargetOptions> TargetOpts;

  /// Options controlling the preprocessor (aside from \#include handling).
  std::shared_ptr<clang::PreprocessorOptions> PreprocessorOpts;

  CompilerInvocationBase();
  CompilerInvocationBase(const CompilerInvocationBase &X);
  CompilerInvocationBase &operator=(const CompilerInvocationBase &) = delete;
  ~CompilerInvocationBase();

  TargetOptions &getTargetOpts() { return *TargetOpts.get(); }

  clang::PreprocessorOptions &getPreprocessorOpts() {
    return *PreprocessorOpts;
  }

  const clang::PreprocessorOptions &getPreprocessorOpts() const {
    return *PreprocessorOpts;
  }
};

/// Helper class for holding the data necessary to invoke the compiler.
///
/// This class is designed to represent an abstract "invocation" of the
/// compiler, including data such as the include paths, the code generation
/// options, the warning flags, and so on.
class CompilerInvocation : public CompilerInvocationBase {
  /// Options controlling the frontend itself.
  FrontendOptions FrontendOpts;

public:
  CompilerInvocation() {}

  FrontendOptions &getFrontendOpts() { return FrontendOpts; }
  const FrontendOptions &getFrontendOpts() const { return FrontendOpts; }
};

} // namespace latino

#endif