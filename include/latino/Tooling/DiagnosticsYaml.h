//===-- DiagnosticsYaml.h -- Serialiazation for Diagnosticss ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines the structure of a YAML document for serializing
/// diagnostics.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_TOOLING_DIAGNOSTICSYAML_H
#define LLVM_LATINO_TOOLING_DIAGNOSTICSYAML_H

#include "latino/Tooling/Core/Diagnostic.h"
#include "latino/Tooling/ReplacementsYaml.h"
#include "llvm/Support/YAMLTraits.h"
#include <string>

LLVM_YAML_IS_SEQUENCE_VECTOR(latino::tooling::Diagnostic)
LLVM_YAML_IS_SEQUENCE_VECTOR(latino::tooling::DiagnosticMessage)
LLVM_YAML_IS_SEQUENCE_VECTOR(latino::tooling::FileByteRange)

namespace llvm {
namespace yaml {

template <> struct MappingTraits<latino::tooling::FileByteRange> {
  static void mapping(IO &Io, latino::tooling::FileByteRange &R) {
    Io.mapRequired("FilePath", R.FilePath);
    Io.mapRequired("FileOffset", R.FileOffset);
    Io.mapRequired("Length", R.Length);
  }
};

template <> struct MappingTraits<latino::tooling::DiagnosticMessage> {
  static void mapping(IO &Io, latino::tooling::DiagnosticMessage &M) {
    Io.mapRequired("Message", M.Message);
    Io.mapOptional("FilePath", M.FilePath);
    Io.mapOptional("FileOffset", M.FileOffset);
    std::vector<latino::tooling::Replacement> Fixes;
    for (auto &Replacements : M.Fix) {
      for (auto &Replacement : Replacements.second)
        Fixes.push_back(Replacement);
    }
    Io.mapRequired("Replacements", Fixes);
    for (auto &Fix : Fixes) {
      llvm::Error Err = M.Fix[Fix.getFilePath()].add(Fix);
      if (Err) {
        // FIXME: Implement better conflict handling.
        llvm::errs() << "Fix conflicts with existing fix: "
                     << llvm::toString(std::move(Err)) << "\n";
      }
    }
  }
};

template <> struct MappingTraits<latino::tooling::Diagnostic> {
  /// Helper to (de)serialize a Diagnostic since we don't have direct
  /// access to its data members.
  class NormalizedDiagnostic {
  public:
    NormalizedDiagnostic(const IO &)
        : DiagLevel(latino::tooling::Diagnostic::Level::Warning) {}

    NormalizedDiagnostic(const IO &, const latino::tooling::Diagnostic &D)
        : DiagnosticName(D.DiagnosticName), Message(D.Message), Notes(D.Notes),
          DiagLevel(D.DiagLevel), BuildDirectory(D.BuildDirectory),
          Ranges(D.Ranges) {}

    latino::tooling::Diagnostic denormalize(const IO &) {
      return latino::tooling::Diagnostic(DiagnosticName, Message, Notes,
                                        DiagLevel, BuildDirectory, Ranges);
    }

    std::string DiagnosticName;
    latino::tooling::DiagnosticMessage Message;
    SmallVector<latino::tooling::DiagnosticMessage, 1> Notes;
    latino::tooling::Diagnostic::Level DiagLevel;
    std::string BuildDirectory;
    SmallVector<latino::tooling::FileByteRange, 1> Ranges;
  };

  static void mapping(IO &Io, latino::tooling::Diagnostic &D) {
    MappingNormalization<NormalizedDiagnostic, latino::tooling::Diagnostic> Keys(
        Io, D);
    Io.mapRequired("DiagnosticName", Keys->DiagnosticName);
    Io.mapRequired("DiagnosticMessage", Keys->Message);
    Io.mapOptional("Notes", Keys->Notes);
    Io.mapOptional("Level", Keys->DiagLevel);
    Io.mapOptional("BuildDirectory", Keys->BuildDirectory);
    Io.mapOptional("Ranges", Keys->Ranges);
  }
};

/// Specialized MappingTraits to describe how a
/// TranslationUnitDiagnostics is (de)serialized.
template <> struct MappingTraits<latino::tooling::TranslationUnitDiagnostics> {
  static void mapping(IO &Io, latino::tooling::TranslationUnitDiagnostics &Doc) {
    Io.mapRequired("MainSourceFile", Doc.MainSourceFile);
    Io.mapRequired("Diagnostics", Doc.Diagnostics);
  }
};

template <> struct ScalarEnumerationTraits<latino::tooling::Diagnostic::Level> {
  static void enumeration(IO &IO, latino::tooling::Diagnostic::Level &Value) {
    IO.enumCase(Value, "Warning", latino::tooling::Diagnostic::Warning);
    IO.enumCase(Value, "Error", latino::tooling::Diagnostic::Error);
  }
};

} // end namespace yaml
} // end namespace llvm

#endif // LLVM_LATINO_TOOLING_DIAGNOSTICSYAML_H
