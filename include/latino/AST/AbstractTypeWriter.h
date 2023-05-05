//==--- AbstractTypeWriter.h - Abstract serialization for types -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LATINO_AST_ABSTRACTTYPEWRITER_H
#define LATINO_AST_ABSTRACTTYPEWRITER_H

#include "latino/AST/Type.h"
#include "latino/AST/AbstractBasicWriter.h"
#include "latino/AST/DeclObjC.h"

namespace latino {
namespace serialization {

// template <class PropertyWriter>
// class AbstractTypeWriter {
// public:
//   AbstractTypeWriter(PropertyWriter &W);
//   void write(QualType type);
// };
//
// The actual class is auto-generated; see latinoASTPropertiesEmitter.cpp.
#include "latino/AST/AbstractTypeWriter.inc"

} // end namespace serialization
} // end namespace latino

#endif
