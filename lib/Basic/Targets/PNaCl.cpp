//===--- PNaCl.cpp - Implement PNaCl target feature support ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements PNaCl TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "PNaCl.h"
#include "latino/Basic/MacroBuilder.h"

using namespace latino;
using namespace latino::targets;

ArrayRef<const char *> PNaClTargetInfo::getGCCRegNames() const { return None; }

ArrayRef<TargetInfo::GCCRegAlias> PNaClTargetInfo::getGCCRegAliases() const {
  return None;
}

void PNaClTargetInfo::getArchDefines(const LangOptions &Opts,
                                     MacroBuilder &Builder) const {
  Builder.defineMacro("__le32__");
  Builder.defineMacro("__pnacl__");
}
