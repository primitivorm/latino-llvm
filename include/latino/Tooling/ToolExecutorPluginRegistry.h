//===- ToolExecutorPluginRegistry.h -----------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_TOOLING_TOOLEXECUTORPLUGINREGISTRY_H
#define LLVM_LATINO_TOOLING_TOOLEXECUTORPLUGINREGISTRY_H

#include "latino/Tooling/Execution.h"
#include "llvm/Support/Registry.h"

namespace latino {
namespace tooling {

using ToolExecutorPluginRegistry = llvm::Registry<ToolExecutorPlugin>;

} // namespace tooling
} // namespace latino

#endif // LLVM_LATINO_TOOLING_TOOLEXECUTORPLUGINREGISTRY_H
