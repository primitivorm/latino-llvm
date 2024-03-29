//===--- VE.h - VE-specific Tool Helpers ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_LIB_DRIVER_TOOLCHAINS_ARCH_VE_H
#define LLVM_LATINO_LIB_DRIVER_TOOLCHAINS_ARCH_VE_H

#include "latino/Driver/Driver.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Option/Option.h"
#include <string>
#include <vector>

namespace latino {
namespace driver {
namespace tools {
namespace ve {

void getVETargetFeatures(const Driver &D, const llvm::opt::ArgList &Args,
                         std::vector<llvm::StringRef> &Features);
const char *getVEAsmModeForCPU(llvm::StringRef Name,
                               const llvm::Triple &Triple);

} // end namespace ve
} // namespace tools
} // end namespace driver
} // end namespace latino

#endif // LLVM_LATINO_LIB_DRIVER_TOOLCHAINS_ARCH_VE_H
