//===- LangOptions.h - C Language Family Language Options -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Defines the latino::LangOptions interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_BASIC_LANGOPTIONS_H
#define LLVM_LATINO_BASIC_LANGOPTIONS_H

#include "llvm/ADT/FloatingPointMode.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Triple.h"
#include <string>
#include <vector>

namespace latino {

/// Bitfields of LangOptions, split out from LangOptions in order to ensure that
/// this large collection of bitfields is a trivial class type.
class LangOptionsBase {
public:
  // Define simple language options (with no accessors).
#define LANGOPT(Name, Bits, Default, Description) unsigned Name : Bits;
#define ENUM_LANGOPT(Name, Type, Bits, Default, Description)
#include "latino/Basic/LangOptions.def"

protected:
  // Define language options of enumeration type. These are private, and will
  // have accessors (below).

#define LANGOPT(Name, Bits, Default, Description)
#define ENUM_LANGOPT(Name, Type, Bits, Default, Description)                   \
  unsigned Name : Bits;
#include "latino/Basic/LangOptions.def"
};

class LangOptions : public LangOptionsBase {
public:
  LangOptions();

// Define accessors/mutators for language options of enumeration type.
#define LANGOPT(Name, Bits, Default, Description)
#define ENUM_LANGOPT(Name, Type, Bits, Default, Description)                   \
  Type get##Name() const { return static_cast<Type>(Name); }                   \
  void set##Name(Type Value) { Name = static_cast<unsigned>(Value); }

#include "latino/Basic/LangOptions.def"
};

/// Describes the kind of translation unit being processed.
enum TranslationUnitKind {
  /// The translation unit is a complete translation unit.
  TU_Complete,

  /// The translation unit is a prefix to a translation unit, and is
  /// not complete.
  TU_Prefix,

  /// The translation unit is a module.
  TU_Module
};

} // namespace latino

#endif