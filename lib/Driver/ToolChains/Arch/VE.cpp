//===--- VE.cpp - Tools Implementations -------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "VE.h"
#include "latino/Driver/Driver.h"
#include "latino/Driver/DriverDiagnostic.h"
#include "latino/Driver/Options.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Option/ArgList.h"

using namespace latino::driver;
using namespace latino::driver::tools;
using namespace latino;
using namespace llvm::opt;

const char *ve::getVEAsmModeForCPU(StringRef Name, const llvm::Triple &Triple) {
  return "";
}

void ve::getVETargetFeatures(const Driver &D, const ArgList &Args,
                             std::vector<StringRef> &Features) {}
