//===--- AIX.h - AIX ToolChain Implementations ------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_LIB_DRIVER_TOOLCHAINS_AIX_H
#define LLVM_LATINO_LIB_DRIVER_TOOLCHAINS_AIX_H

#include "latino/Driver/Tool.h"
#include "latino/Driver/ToolChain.h"

namespace latino {
namespace driver {
namespace tools {

/// aix -- Directly call system default assembler and linker.
namespace aix {

class LLVM_LIBRARY_VISIBILITY Assembler : public Tool {
public:
  Assembler(const ToolChain &TC) : Tool("aix::Assembler", "assembler", TC) {}

  bool hasIntegratedCPP() const override { return false; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

class LLVM_LIBRARY_VISIBILITY Linker : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("aix::Linker", "linker", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

} // end namespace aix

} // end namespace tools
} // end namespace driver
} // end namespace latino

namespace latino {
namespace driver {
namespace toolchains {

class LLVM_LIBRARY_VISIBILITY AIX : public ToolChain {
public:
  AIX(const Driver &D, const llvm::Triple &Triple,
      const llvm::opt::ArgList &Args);

  bool isPICDefault() const override { return true; }
  bool isPIEDefault() const override { return false; }
  bool isPICDefaultForced() const override { return true; }

  void
  AddClangSystemIncludeArgs(const llvm::opt::ArgList &DriverArgs,
                            llvm::opt::ArgStringList &CC1Args) const override;

protected:
  Tool *buildAssembler() const override;
  Tool *buildLinker() const override;

private:
  llvm::StringRef GetHeaderSysroot(const llvm::opt::ArgList &DriverArgs) const;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace latino

#endif // LLVM_LATINO_LIB_DRIVER_TOOLCHAINS_AIX_H
