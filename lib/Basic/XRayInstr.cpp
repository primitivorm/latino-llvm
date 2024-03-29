//===--- XRayInstr.cpp ------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This is part of XRay, a function call instrumentation system.
//
//===----------------------------------------------------------------------===//

#include "latino/Basic/XRayInstr.h"
#include "llvm/ADT/StringSwitch.h"

namespace latino {

XRayInstrMask parseXRayInstrValue(StringRef Value) {
  XRayInstrMask ParsedKind =
      llvm::StringSwitch<XRayInstrMask>(Value)
          .Case("all", XRayInstrKind::All)
          .Case("custom", XRayInstrKind::Custom)
          .Case("function",
                XRayInstrKind::FunctionEntry | XRayInstrKind::FunctionExit)
          .Case("function-entry", XRayInstrKind::FunctionEntry)
          .Case("function-exit", XRayInstrKind::FunctionExit)
          .Case("typed", XRayInstrKind::Typed)
          .Case("none", XRayInstrKind::None)
          .Default(XRayInstrKind::None);
  return ParsedKind;
}

} // namespace latino
