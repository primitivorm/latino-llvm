//==--- AbstractTypeReader.h - Abstract deserialization for types ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LATINO_AST_ABSTRACTTYPEREADER_H
#define LATINO_AST_ABSTRACTTYPEREADER_H

#include "latino/AST/Type.h"
#include "latino/AST/AbstractBasicReader.h"

namespace latino {
namespace serialization {

// template <class PropertyReader>
// class AbstractTypeReader {
// public:
//   AbstractTypeReader(PropertyReader &W);
//   QualType read(Type::TypeClass kind);
// };
//
// The actual class is auto-generated; see latinoASTPropertiesEmitter.cpp.
#include "clang/AST/AbstractTypeReader.inc"

} // end namespace serialization
} // end namespace latino

#endif
