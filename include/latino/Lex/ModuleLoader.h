//===- ModuleLoader.h - Module Loader Interface -----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the ModuleLoader interface, which is responsible for
//  loading named modules.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_LEX_MODULELOADER_H
#define LLVM_LATINO_LEX_MODULELOADER_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include <utility>

namespace latino {

/// Abstract interface for a module loader.
///
/// This abstract interface describes a module loader, which is responsible
/// for resolving a module name (e.g., "std") to an actual module file, and
/// then loading that module.
class ModuleLoader {
  // Building a module if true.
  bool BuildingModule;

public:
  explicit ModuleLoader(bool BuildingModule = false)
      : BuildingModule(BuildingModule) {}

  virtual ~ModuleLoader();
};

/// A module loader that doesn't know how to create or load modules.
class TrivialModuleLoader : public ModuleLoader {
public:
};
} // namespace latino

#endif