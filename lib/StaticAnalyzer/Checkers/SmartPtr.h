//=== SmartPtr.h - Tracking smart pointer state. -------------------*- C++ -*-//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Defines inter-checker API for the smart pointer modeling. It allows
// dependent checkers to figure out if an smart pointer is null or not.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_LIB_STATICANALYZER_CHECKERS_SMARTPTR_H
#define LLVM_LATINO_LIB_STATICANALYZER_CHECKERS_SMARTPTR_H

#include "latino/StaticAnalyzer/Core/PathSensitive/CallEvent.h"

namespace latino {
namespace ento {
namespace smartptr {

/// Returns true if the event call is on smart pointer.
bool isStdSmartPtrCall(const CallEvent &Call);

/// Returns whether the smart pointer is null or not.
bool isNullSmartPtr(const ProgramStateRef State, const MemRegion *ThisRegion);

} // namespace smartptr
} // namespace ento
} // namespace latino

#endif // LLVM_LATINO_LIB_STATICANALYZER_CHECKERS_SMARTPTR_H
