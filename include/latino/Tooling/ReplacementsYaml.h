//===-- ReplacementsYaml.h -- Serialiazation for Replacements ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines the structure of a YAML document for serializing
/// replacements.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_TOOLING_REPLACEMENTSYAML_H
#define LLVM_LATINO_TOOLING_REPLACEMENTSYAML_H

#include "latino/Tooling/Refactoring.h"
#include "llvm/Support/YAMLTraits.h"
#include <string>

LLVM_YAML_IS_SEQUENCE_VECTOR(latino::tooling::Replacement)

namespace llvm {
namespace yaml {

/// Specialized MappingTraits to describe how a Replacement is
/// (de)serialized.
template <> struct MappingTraits<latino::tooling::Replacement> {
  /// Helper to (de)serialize a Replacement since we don't have direct
  /// access to its data members.
  struct NormalizedReplacement {
    NormalizedReplacement(const IO &)
        : FilePath(""), Offset(0), Length(0), ReplacementText("") {}

    NormalizedReplacement(const IO &, const latino::tooling::Replacement &R)
        : FilePath(R.getFilePath()), Offset(R.getOffset()),
          Length(R.getLength()), ReplacementText(R.getReplacementText()) {}

    latino::tooling::Replacement denormalize(const IO &) {
      return latino::tooling::Replacement(FilePath, Offset, Length,
                                         ReplacementText);
    }

    std::string FilePath;
    unsigned int Offset;
    unsigned int Length;
    std::string ReplacementText;
  };

  static void mapping(IO &Io, latino::tooling::Replacement &R) {
    MappingNormalization<NormalizedReplacement, latino::tooling::Replacement>
    Keys(Io, R);
    Io.mapRequired("FilePath", Keys->FilePath);
    Io.mapRequired("Offset", Keys->Offset);
    Io.mapRequired("Length", Keys->Length);
    Io.mapRequired("ReplacementText", Keys->ReplacementText);
  }
};

/// Specialized MappingTraits to describe how a
/// TranslationUnitReplacements is (de)serialized.
template <> struct MappingTraits<latino::tooling::TranslationUnitReplacements> {
  static void mapping(IO &Io,
                      latino::tooling::TranslationUnitReplacements &Doc) {
    Io.mapRequired("MainSourceFile", Doc.MainSourceFile);
    Io.mapRequired("Replacements", Doc.Replacements);
  }
};
} // end namespace yaml
} // end namespace llvm

#endif
