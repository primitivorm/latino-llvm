//===- Module.h - Describe a module -----------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Defines the latino::Module class, which describes a module in the
/// source code.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_BASIC_MODULE_H
#define LLVM_LATINO_BASIC_MODULE_H

#include "clang/Basic/SourceLocation.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/PointerIntPair.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/iterator_range.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <ctime>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace llvm {

class raw_ostream;

} // namespace llvm

namespace latino {

    /// Describes a module or submodule.
class Module {
};

}

#endif
