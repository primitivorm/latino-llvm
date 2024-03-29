//===--- ARM.h - ARM-specific (not AArch64) Tool Helpers --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_LIB_DRIVER_TOOLCHAINS_ARCH_ARM_H
#define LLVM_LATINO_LIB_DRIVER_TOOLCHAINS_ARCH_ARM_H

#include "latino/Driver/ToolChain.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Option/Option.h"
#include "llvm/Support/TargetParser.h"
#include <string>
#include <vector>

namespace latino {
namespace driver {
namespace tools {
namespace arm {

std::string getARMTargetCPU(StringRef CPU, llvm::StringRef Arch,
                            const llvm::Triple &Triple);
const std::string getARMArch(llvm::StringRef Arch, const llvm::Triple &Triple);
StringRef getARMCPUForMArch(llvm::StringRef Arch, const llvm::Triple &Triple);
llvm::ARM::ArchKind getLLVMArchKindForARM(StringRef CPU, StringRef Arch,
                                          const llvm::Triple &Triple);
StringRef getLLVMArchSuffixForARM(llvm::StringRef CPU, llvm::StringRef Arch,
                                  const llvm::Triple &Triple);

void appendBE8LinkFlag(const llvm::opt::ArgList &Args,
                       llvm::opt::ArgStringList &CmdArgs,
                       const llvm::Triple &Triple);
enum class ReadTPMode {
  Invalid,
  Soft,
  Cp15,
};

enum class FloatABI {
  Invalid,
  Soft,
  SoftFP,
  Hard,
};

FloatABI getARMFloatABI(const ToolChain &TC, const llvm::opt::ArgList &Args);
FloatABI getARMFloatABI(const Driver &D, const llvm::Triple &Triple,
                        const llvm::opt::ArgList &Args);
ReadTPMode getReadTPMode(const Driver &D, const llvm::opt::ArgList &Args);

bool useAAPCSForMachO(const llvm::Triple &T);
void getARMArchCPUFromArgs(const llvm::opt::ArgList &Args,
                           llvm::StringRef &Arch, llvm::StringRef &CPU,
                           bool FromAs = false);
void getARMTargetFeatures(const Driver &D, const llvm::Triple &Triple,
                          const llvm::opt::ArgList &Args,
                          llvm::opt::ArgStringList &CmdArgs,
                          std::vector<llvm::StringRef> &Features, bool ForAS);
int getARMSubArchVersionNumber(const llvm::Triple &Triple);
bool isARMMProfile(const llvm::Triple &Triple);

} // end namespace arm
} // end namespace tools
} // end namespace driver
} // end namespace latino

#endif // LLVM_LATINO_LIB_DRIVER_TOOLCHAINS_ARCH_ARM_H
