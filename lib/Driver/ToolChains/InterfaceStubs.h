//===---  InterfaceStubs.cpp - Base InterfaceStubs Implementations C++  ---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_LIB_DRIVER_TOOLCHAINS_IFS_H
#define LLVM_LATINO_LIB_DRIVER_TOOLCHAINS_IFS_H

#include "latino/Driver/Tool.h"
#include "latino/Driver/ToolChain.h"

namespace latino {
namespace driver {
namespace tools {
namespace ifstool {
class LLVM_LIBRARY_VISIBILITY Merger : public Tool {
public:
  Merger(const ToolChain &TC) : Tool("IFS::Merger", "llvm-ifs", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return false; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};
} // end namespace ifstool
} // end namespace tools
} // end namespace driver
} // end namespace latino

#endif // LLVM_LATINO_LIB_DRIVER_TOOLCHAINS_IFS_H
