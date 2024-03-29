//===--- USRLocFinder.h - Clang refactoring library -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Provides functionality for finding all instances of a USR in a given
/// AST.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_TOOLING_REFACTOR_RENAME_USR_LOC_FINDER_H
#define LLVM_LATINO_TOOLING_REFACTOR_RENAME_USR_LOC_FINDER_H

#include "latino/AST/AST.h"
#include "latino/Tooling/Core/Replacement.h"
#include "latino/Tooling/Refactoring/AtomicChange.h"
#include "latino/Tooling/Refactoring/Rename/SymbolOccurrences.h"
#include "llvm/ADT/StringRef.h"
#include <string>
#include <vector>

namespace latino {
namespace tooling {

/// Create atomic changes for renaming all symbol references which are
/// identified by the USRs set to a given new name.
///
/// \param USRs The set containing USRs of a particular old symbol.
/// \param NewName The new name to replace old symbol name.
/// \param TranslationUnitDecl The translation unit declaration.
///
/// \return Atomic changes for renaming.
std::vector<tooling::AtomicChange>
createRenameAtomicChanges(llvm::ArrayRef<std::string> USRs,
                          llvm::StringRef NewName, Decl *TranslationUnitDecl);

/// Finds the symbol occurrences for the symbol that's identified by the given
/// USR set.
///
/// \return SymbolOccurrences that can be converted to AtomicChanges when
/// renaming.
SymbolOccurrences getOccurrencesOfUSRs(ArrayRef<std::string> USRs,
                                       StringRef PrevName, Decl *Decl);

} // end namespace tooling
} // end namespace latino

#endif // LLVM_LATINO_TOOLING_REFACTOR_RENAME_USR_LOC_FINDER_H
