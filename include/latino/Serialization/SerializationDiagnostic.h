//===--- SerializationDiagnostic.h - Serialization Diagnostics -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LATINO_SERIALIZATION_SERIALIZATIONDIAGNOSTIC_H
#define LATINO_SERIALIZATION_SERIALIZATIONDIAGNOSTIC_H

#include "latino/Basic/Diagnostic.h"

namespace latino {
namespace diag {
enum {
#define DIAG(ENUM, FLAGS, DEFAULT_MAPPING, DESC, GROUP, SFINAE, NOWERROR,      \
             SHOWINSYSHEADER, CATEGORY)                                        \
  ENUM,
#define SERIALIZATIONSTART
#include "latino/Basic/DiagnosticSerializationKinds.inc"
#undef DIAG
  NUM_BUILTIN_SERIALIZATION_DIAGNOSTICS
};
} // end namespace diag
} // end namespace latino

#endif
