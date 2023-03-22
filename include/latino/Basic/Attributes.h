//===--- Attributes.h - Attributes header -----------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_BASIC_ATTRIBUTES_H
#define LLVM_LATINO_BASIC_ATTRIBUTES_H

#include "latino/Basic/LangOptions.h"
#include "latino/Basic/TargetInfo.h"

namespace latino {

class IdentifierInfo;

enum class AttrSyntax {
  /// Is the identifier known as a GNU-style attribute?
  GNU,
  /// Is the identifier known as a __declspec-style attribute?
  Declspec,
  /// Is the identifier known as a [] Microsoft-style attribute?
  Microsoft,
  // Is the identifier known as a C++-style attribute?
  CXX,
  // Is the identifier known as a C-style attribute?
  C,
  // Is the identifier known as a pragma attribute?
  Pragma
};

/// Return the version number associated with the attribute if we
/// recognize and implement the attribute specified by the given information.
int hasAttribute(AttrSyntax Syntax, const IdentifierInfo *Scope,
                 const IdentifierInfo *Attr, const TargetInfo &Target,
                 const LangOptions &LangOpts);

} // end namespace latino

#endif // LLVM_LATINO_BASIC_ATTRIBUTES_H
