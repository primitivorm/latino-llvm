//===--- PPCLinux.h - PowerPC ToolChain Implementations ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_LIB_DRIVER_TOOLCHAINS_PPC_LINUX_H
#define LLVM_LATINO_LIB_DRIVER_TOOLCHAINS_PPC_LINUX_H

#include "Linux.h"

namespace latino {
namespace driver {
namespace toolchains {

class LLVM_LIBRARY_VISIBILITY PPCLinuxToolChain : public Linux {
public:
  PPCLinuxToolChain(const Driver &D, const llvm::Triple &Triple,
                    const llvm::opt::ArgList &Args)
      : Linux(D, Triple, Args) {}

  void
  AddClangSystemIncludeArgs(const llvm::opt::ArgList &DriverArgs,
                            llvm::opt::ArgStringList &CC1Args) const override;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace latino

#endif // LLVM_LATINO_LIB_DRIVER_TOOLCHAINS_PPC_LINUX_H
