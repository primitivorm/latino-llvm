//===--- XRayInstr.h --------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Defines the latino::XRayInstrKind enum.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_BASIC_XRAYINSTR_H
#define LLVM_LATINO_BASIC_XRAYINSTR_H

#include "latino/Basic/LLVM.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MathExtras.h"
#include <cassert>
#include <cstdint>

namespace latino {

using XRayInstrMask = uint32_t;

namespace XRayInstrKind {

// TODO: Auto-generate these as we add more instrumentation kinds.
enum XRayInstrOrdinal : XRayInstrMask {
  XRIO_FunctionEntry,
  XRIO_FunctionExit,
  XRIO_Custom,
  XRIO_Typed,
  XRIO_Count
};

constexpr XRayInstrMask None = 0;
constexpr XRayInstrMask FunctionEntry = 1U << XRIO_FunctionEntry;
constexpr XRayInstrMask FunctionExit = 1U << XRIO_FunctionExit;
constexpr XRayInstrMask Custom = 1U << XRIO_Custom;
constexpr XRayInstrMask Typed = 1U << XRIO_Typed;
constexpr XRayInstrMask All = FunctionEntry | FunctionExit | Custom | Typed;

} // namespace XRayInstrKind

struct XRayInstrSet {
  bool has(XRayInstrMask K) const {
    assert(llvm::isPowerOf2_32(K));
    return Mask & K;
  }

  bool hasOneOf(XRayInstrMask K) const { return Mask & K; }

  void set(XRayInstrMask K, bool Value) {
    Mask = Value ? (Mask | K) : (Mask & ~K);
  }

  void clear(XRayInstrMask K = XRayInstrKind::All) { Mask &= ~K; }

  bool empty() const { return Mask == 0; }

  bool full() const { return Mask == XRayInstrKind::All; }

  XRayInstrMask Mask = 0;
};

XRayInstrMask parseXRayInstrValue(StringRef Value);

} // namespace latino

#endif // LLVM_LATINO_BASIC_XRAYINSTR_H
