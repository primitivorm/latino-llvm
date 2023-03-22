//===--- AllDiagnostics.h - Aggregate Diagnostic headers --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Includes all the separate Diagnostic headers & some related helpers.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_BASIC_ALLDIAGNOSTICS_H
#define LLVM_LATINO_BASIC_ALLDIAGNOSTICS_H

#include "latino/Basic/DiagnosticAST.h"
#include "latino/Basic/DiagnosticAnalysis.h"
#include "latino/Basic/DiagnosticComment.h"
#include "latino/Basic/DiagnosticCrossTU.h"
#include "latino/Basic/DiagnosticDriver.h"
#include "latino/Basic/DiagnosticFrontend.h"
#include "latino/Basic/DiagnosticLex.h"
#include "latino/Basic/DiagnosticParse.h"
#include "latino/Basic/DiagnosticSema.h"
#include "latino/Basic/DiagnosticSerialization.h"
#include "latino/Basic/DiagnosticRefactoring.h"

namespace latino {
template <size_t SizeOfStr, typename FieldType>
class StringSizerHelper {
  static_assert(SizeOfStr <= FieldType(~0U), "Field too small!");
public:
  enum { Size = SizeOfStr };
};
} // end namespace latino

#define STR_SIZE(str, fieldTy) latino::StringSizerHelper<sizeof(str)-1, \
                                                        fieldTy>::Size

#endif
