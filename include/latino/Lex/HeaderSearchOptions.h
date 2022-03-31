//===- HeaderSearchOptions.h ------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LEX_HEADERSEARCHOPTIONS_H
#define LLVM_CLANG_LEX_HEADERSEARCHOPTIONS_H

#include "latino/Basic/LLVM.h"

#include "llvm/ADT/CachedHashString.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/StringRef.h"
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace latino {

/// HeaderSearchOptions - Helper class for storing options related to the
/// initialization of the HeaderSearch object.
class HeaderSearchOptions {

  /// If non-empty, the directory to use as a "virtual system root" for include
  /// paths.
  std::string Sysroot;

public:
  HeaderSearchOptions(StringRef _Sysroot = "/") : Sysroot(_Sysroot) {}
};

} // namespace latino

#endif