//===- Decl.cpp - Declaration AST Node Implementation ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the Decl subclasses.
//
//===----------------------------------------------------------------------===//
#include "latino/AST/Decl.h"

#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/None.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>

using namespace latino;

std::string NamedDecl::getQualifiedNameAsString() const {
  std::string QualName;
  llvm::raw_string_ostream OS(QualName);
  //   printQualifiedName(OS, getASTContext().getPrintingPolicy());
  return OS.str();
}

// void NamedDecl::printQualifiedName(raw_ostream &OS) const {
//   printQualifiedName(OS, getASTContext().getPrintingPolicy());
// }

// void NamedDecl::printQualifiedName(raw_ostream &OS,
//                                    const PrintingPolicy &P) const {
//   if (getDeclContext()->isFunctionOrMethod()) {
//     // We do not print '(anonymous)' for function parameters without name.
//     printName(OS);
//     return;
//   }

//   printNestedNameSpecifier(OS, P);
//   if (getDeclName())
//     OS << *this;
//   else {
//     // Give the printName override a chance to pick a different name before
//     we
//     // fall back to "(anonymous)".
//     SmallString<64> NameBuffer;
//   }
// }