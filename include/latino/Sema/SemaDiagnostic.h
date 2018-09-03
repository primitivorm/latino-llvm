//===--- DiagnosticSema.h - Diagnostics for libsema -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LATINO_SEMA_SEMADIAGNOSTIC_H
#define LATINO_SEMA_SEMADIAGNOSTIC_H

#include "latino/Basic/Diagnostic.h"

namespace latino {
namespace diag {
enum {
#define DIAG(ENUM, FLAGS, DEFAULT_MAPPING, DESC, GROUP, SFINAE, NOWERROR,      \
             SHOWINSYSHEADER, CATEGORY)                                        \
  ENUM,
#define SEMASTART
#include "latino/Basic/DiagnosticSemaKinds.inc"
#undef DIAG
  NUM_BUILTIN_SEMA_DIAGNOSTICS
};
} // end namespace diag
} // end namespace latino

#endif /* LATINO_SEMA_SEMADIAGNOSTIC_H */
