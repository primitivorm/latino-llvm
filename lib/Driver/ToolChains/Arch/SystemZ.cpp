//===--- SystemZ.cpp - SystemZ Helpers for Tools ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SystemZ.h"
#include "latino/Config/config.h"
#include "latino/Driver/DriverDiagnostic.h"
#include "latino/Driver/Options.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/Host.h"

using namespace latino::driver;
using namespace latino::driver::tools;
using namespace latino;
using namespace llvm::opt;

systemz::FloatABI systemz::getSystemZFloatABI(const Driver &D,
                                              const ArgList &Args) {
  // Hard float is the default.
  systemz::FloatABI ABI = systemz::FloatABI::Hard;
  if (Args.hasArg(options::OPT_mfloat_abi_EQ))
    D.Diag(diag::err_drv_unsupported_opt)
      << Args.getLastArg(options::OPT_mfloat_abi_EQ)->getAsString(Args);

  if (Arg *A = Args.getLastArg(latino::driver::options::OPT_msoft_float,
                               options::OPT_mhard_float))
    if (A->getOption().matches(latino::driver::options::OPT_msoft_float))
      ABI = systemz::FloatABI::Soft;

  return ABI;
}

std::string systemz::getSystemZTargetCPU(const ArgList &Args) {
  if (const Arg *A = Args.getLastArg(latino::driver::options::OPT_march_EQ)) {
    llvm::StringRef CPUName = A->getValue();

    if (CPUName == "native") {
      std::string CPU = std::string(llvm::sys::getHostCPUName());
      if (!CPU.empty() && CPU != "generic")
        return CPU;
      else
        return "";
    }

    return std::string(CPUName);
  }
  return LATINO_SYSTEMZ_DEFAULT_ARCH;
}

void systemz::getSystemZTargetFeatures(const Driver &D, const ArgList &Args,
                                       std::vector<llvm::StringRef> &Features) {
  // -m(no-)htm overrides use of the transactional-execution facility.
  if (Arg *A = Args.getLastArg(options::OPT_mhtm, options::OPT_mno_htm)) {
    if (A->getOption().matches(options::OPT_mhtm))
      Features.push_back("+transactional-execution");
    else
      Features.push_back("-transactional-execution");
  }
  // -m(no-)vx overrides use of the vector facility.
  if (Arg *A = Args.getLastArg(options::OPT_mvx, options::OPT_mno_vx)) {
    if (A->getOption().matches(options::OPT_mvx))
      Features.push_back("+vector");
    else
      Features.push_back("-vector");
  }

  systemz::FloatABI FloatABI = systemz::getSystemZFloatABI(D, Args);
  if (FloatABI == systemz::FloatABI::Soft)
    Features.push_back("+soft-float");
}
