//===--- XCore.cpp - Implement XCore target feature support ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements XCore TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "XCore.h"
#include "latino/Basic/Builtins.h"
#include "latino/Basic/MacroBuilder.h"
#include "latino/Basic/TargetBuiltins.h"

using namespace latino;
using namespace latino::targets;

const Builtin::Info XCoreTargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, nullptr},
#define LIBBUILTIN(ID, TYPE, ATTRS, HEADER)                                    \
  {#ID, TYPE, ATTRS, HEADER, ALL_LANGUAGES, nullptr},
#include "latino/Basic/BuiltinsXCore.def"
};

void XCoreTargetInfo::getTargetDefines(const LangOptions &Opts,
                                       MacroBuilder &Builder) const {
  Builder.defineMacro("__XS1B__");
}

ArrayRef<Builtin::Info> XCoreTargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfo, latino::XCore::LastTSBuiltin -
                                             Builtin::FirstTSBuiltin);
}
