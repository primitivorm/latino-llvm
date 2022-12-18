//===--- LangStandard.h -----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_BASIC_LANGSTANDARD_H
#define LLVM_LATINO_BASIC_LANGSTANDARD_H

#include "latino/Basic/LLVM.h"
#include "llvm/ADT/StringRef.h"

namespace latino {
/// The language for the input, used to select and validate the language
/// standard and possible actions.
enum class Language : uint8_t {
  Unknown,

  /// Assembly: we accept this only so that we can preprocess it.
  Asm,

  /// LLVM IR: we accept this so that we can run the optimizer on it,
  /// and compile it to assembly or object code.
  LLVM_IR,

  ///@{ Languages that the frontend can parse and compile.
  Latino
  ///@}
};
} // namespace latino

#endif