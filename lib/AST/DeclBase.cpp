//===- DeclBase.cpp - Declaration AST Node Implementation -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the Decl and DeclContext classes.
//
//===----------------------------------------------------------------------===//

#include "latino/AST/DeclBase.h"
#include "latino/AST/Decl.h"
#include "latino/AST/DeclCXX.h"
#include "latino/AST/Stmt.h"
#include "latino/AST/Type.h"
#include "latino/Basic/IdentifierTable.h"
#include "latino/Basic/LLVM.h"
#include "latino/Basic/LangOptions.h"
#include "latino/Basic/SourceLocation.h"
#include "latino/Basic/TargetInfo.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/PointerIntPair.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/VersionTuple.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <string>
#include <tuple>
#include <utility>

using namespace latino;

//===----------------------------------------------------------------------===//
//  Statistics
//===----------------------------------------------------------------------===//
#define DECL(DERIVED, BASE) static int n##DERIVED##s = 0;
#define ABSTRACT_DECL(DECL)
#include "latino/AST/DeclNodes.inc"

#define DECL(DERIVED, BASE)                                                    \
  static_assert(alignof(Decl) >= alignof(DERIVED##Decl),                       \
                "Alignment sufficient after objects prepended to " #DERIVED);
#define ABSTRACT_DECL(DECL)
#include "latino/AST/DeclNodes.inc"

bool Decl::StatisticsEnabled = false;
void Decl::EnableStatistics() { StatisticsEnabled = true; }

void Decl::add(Kind K) {
  switch (K) {
#define DECL(DERIVED, BASE)                                                    \
  case DERIVED:                                                                \
    ++n##DERIVED##s;                                                           \
    break;
#define ABSTRACT_DECL(DECL)
#include "latino/AST/DeclNodes.inc"
  }
}

//===----------------------------------------------------------------------===//
// DeclContext Implementation
//===----------------------------------------------------------------------===//

DeclContext::DeclContext(Decl::Kind K) {}