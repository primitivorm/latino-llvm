//===----- Attr.h - Enum values for C Attribute Kinds ----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Defines the latino::attr::Kind enum.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_BASIC_ATTRKINDS_H
#define LLVM_LATINO_BASIC_ATTRKINDS_H

namespace latino {

namespace attr {

// A list of all the recognized kinds of attributes.
enum Kind {
#define ATTR(X) X,
#define ATTR_RANGE(CLASS, FIRST_NAME, LAST_NAME) \
  First##CLASS = FIRST_NAME,                    \
  Last##CLASS = LAST_NAME,
#include "clang/Basic/AttrList.inc"
};

} // end namespace attr
} // end namespace latino

#endif
