//===--- Flang.h - Flang Tool and ToolChain Implementations ====-*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_LIB_DRIVER_TOOLCHAINS_FLANG_H
#define LLVM_LATINO_LIB_DRIVER_TOOLCHAINS_FLANG_H

#include "latino/Driver/Tool.h"
#include "latino/Driver/Action.h"
#include "latino/Driver/Compilation.h"
#include "latino/Driver/ToolChain.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/Compiler.h"

namespace latino {
namespace driver {

namespace tools {

/// Flang compiler tool.
class LLVM_LIBRARY_VISIBILITY Flang : public Tool {
public:
  Flang(const ToolChain &TC);
  ~Flang() override;

  bool hasGoodDiagnostics() const override { return true; }
  bool hasIntegratedAssembler() const override { return true; }
  bool hasIntegratedCPP() const override { return true; }
  bool canEmitIR() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

} // end namespace tools

} // end namespace driver
} // end namespace latino

#endif // LLVM_LATINO_LIB_DRIVER_TOOLCHAINS_FLANG_H
