//===--- MSP430.h - MSP430-specific Tool Helpers ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_LIB_DRIVER_TOOLCHAINS_MSP430_H
#define LLVM_LATINO_LIB_DRIVER_TOOLCHAINS_MSP430_H

#include "Gnu.h"
#include "InputInfo.h"
#include "latino/Driver/Driver.h"
#include "latino/Driver/DriverDiagnostic.h"
#include "latino/Driver/Tool.h"
#include "latino/Driver/ToolChain.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Option/Option.h"

#include <string>
#include <vector>

namespace latino {
namespace driver {
namespace toolchains {

class LLVM_LIBRARY_VISIBILITY MSP430ToolChain : public Generic_ELF {
public:
  MSP430ToolChain(const Driver &D, const llvm::Triple &Triple,
                  const llvm::opt::ArgList &Args);
  void
  AddClangSystemIncludeArgs(const llvm::opt::ArgList &DriverArgs,
                            llvm::opt::ArgStringList &CC1Args) const override;
  void addClangTargetOptions(const llvm::opt::ArgList &DriverArgs,
                             llvm::opt::ArgStringList &CC1Args,
                             Action::OffloadKind) const override;

  bool isPICDefault() const override { return false; }
  bool isPIEDefault() const override { return false; }
  bool isPICDefaultForced() const override { return true; }

protected:
  Tool *buildLinker() const override;

private:
  std::string computeSysRoot() const override;
};

} // end namespace toolchains

namespace tools {
namespace msp430 {

class LLVM_LIBRARY_VISIBILITY Linker : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("MSP430::Linker", "msp430-elf-ld", TC) {}
  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }
  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

void getMSP430TargetFeatures(const Driver &D, const llvm::opt::ArgList &Args,
                             std::vector<llvm::StringRef> &Features);
} // end namespace msp430
} // end namespace tools
} // end namespace driver
} // end namespace latino

#endif // LLVM_LATINO_LIB_DRIVER_TOOLCHAINS_MSP430_H
