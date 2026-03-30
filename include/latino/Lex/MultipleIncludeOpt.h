//===--- MultipleIncludeOpt.h - Header Multiple-Include Optzn ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Defines the MultipleIncludeOpt interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_LEX_MULTIPLEINCLUDEOPT_H
#define LLVM_LATINO_LEX_MULTIPLEINCLUDEOPT_H

#include "clang/Basic/SourceLocation.h"

namespace latino {
/// Implements the simple state machine that the Lexer class uses to
/// detect files subject to the 'multiple-include' optimization.
///
/// The public methods in this class are triggered by various
/// events that occur when a file is lexed, and after the entire file is lexed,
/// information about which macro (if any) controls the header is returned.
class MultipleIncludeOpt {};
} // namespace latino

#endif