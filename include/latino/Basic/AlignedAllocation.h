//===--- AlignedAllocation.h - Aligned Allocation ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Defines a function that returns the minimum OS versions supporting
/// C++17's aligned allocation functions.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_BASIC_ALIGNED_ALLOCATION_H
#define LLVM_LATINO_BASIC_ALIGNED_ALLOCATION_H

#include "llvm/ADT/Triple.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/VersionTuple.h"

namespace latino {

inline llvm::VersionTuple alignedAllocMinVersion(llvm::Triple::OSType OS) {
  switch (OS) {
  default:
    break;
  case llvm::Triple::Darwin:
  case llvm::Triple::MacOSX: // Earliest supporting version is 10.14.
    return llvm::VersionTuple(10U, 14U);
  case llvm::Triple::IOS:
  case llvm::Triple::TvOS: // Earliest supporting version is 11.0.0.
    return llvm::VersionTuple(11U);
  case llvm::Triple::WatchOS: // Earliest supporting version is 4.0.0.
    return llvm::VersionTuple(4U);
  }

  llvm_unreachable("Unexpected OS");
}

} // end namespace latino

#endif // LLVM_LATINO_BASIC_ALIGNED_ALLOCATION_H
