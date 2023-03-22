//===--- Le64.cpp - Implement Le64 target feature support -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements Le64 TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "Le64.h"
#include "Targets.h"
#include "latino/Basic/Builtins.h"
#include "latino/Basic/MacroBuilder.h"
#include "latino/Basic/TargetBuiltins.h"

using namespace latino;
using namespace latino::targets;

const Builtin::Info Le64TargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, nullptr},
#include "latino/Basic/BuiltinsLe64.def"
};

ArrayRef<Builtin::Info> Le64TargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfo, latino::Le64::LastTSBuiltin -
                                             Builtin::FirstTSBuiltin);
}

void Le64TargetInfo::getTargetDefines(const LangOptions &Opts,
                                      MacroBuilder &Builder) const {
  DefineStd(Builder, "unix", Opts);
  defineCPUMacros(Builder, "le64", /*Tuning=*/false);
  Builder.defineMacro("__ELF__");
}
