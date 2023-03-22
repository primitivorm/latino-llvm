//===------- Targets.h - Declare target feature support ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares things required for construction of a TargetInfo object
// from a target triple. Typically individual targets will need to include from
// here in order to get these functions if required.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_LIB_BASIC_TARGETS_H
#define LLVM_LATINO_LIB_BASIC_TARGETS_H

#include "latino/Basic/LangOptions.h"
#include "latino/Basic/MacroBuilder.h"
#include "latino/Basic/TargetInfo.h"
#include "llvm/ADT/StringRef.h"

namespace latino {
namespace targets {

LLVM_LIBRARY_VISIBILITY
latino::TargetInfo *AllocateTarget(const llvm::Triple &Triple,
                                  const latino::TargetOptions &Opts);

/// DefineStd - Define a macro name and standard variants.  For example if
/// MacroName is "unix", then this will define "__unix", "__unix__", and "unix"
/// when in GNU mode.
LLVM_LIBRARY_VISIBILITY
void DefineStd(latino::MacroBuilder &Builder, llvm::StringRef MacroName,
               const latino::LangOptions &Opts);

LLVM_LIBRARY_VISIBILITY
void defineCPUMacros(latino::MacroBuilder &Builder, llvm::StringRef CPUName,
                     bool Tuning = true);

LLVM_LIBRARY_VISIBILITY
void addCygMingDefines(const latino::LangOptions &Opts,
                       latino::MacroBuilder &Builder);
} // namespace targets
} // namespace latino
#endif // LLVM_LATINO_LIB_BASIC_TARGETS_H
