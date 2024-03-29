//===--- FileIndexRecord.h - Index data per file ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_LIB_INDEX_FILEINDEXRECORD_H
#define LLVM_LATINO_LIB_INDEX_FILEINDEXRECORD_H

#include "latino/Basic/SourceLocation.h"
#include "latino/Index/DeclOccurrence.h"
#include "latino/Index/IndexSymbol.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include <vector>

namespace latino {
class IdentifierInfo;

namespace index {

/// Stores the declaration occurrences seen in a particular source or header
/// file of a translation unit
class FileIndexRecord {
private:
  FileID FID;
  bool IsSystem;
  std::vector<DeclOccurrence> Decls;

public:
  FileIndexRecord(FileID FID, bool IsSystem) : FID(FID), IsSystem(IsSystem) {}

  ArrayRef<DeclOccurrence> getDeclOccurrencesSortedByOffset() const {
    return Decls;
  }

  FileID getFileID() const { return FID; }
  bool isSystem() const { return IsSystem; }

  /// Adds an occurrence of the canonical declaration \c D at the supplied
  /// \c Offset
  ///
  /// \param Roles the roles the occurrence fulfills in this position.
  /// \param Offset the offset in the file of this occurrence.
  /// \param D the canonical declaration this is an occurrence of.
  /// \param Relations the set of symbols related to this occurrence.
  void addDeclOccurence(SymbolRoleSet Roles, unsigned Offset, const Decl *D,
                        ArrayRef<SymbolRelation> Relations);
  void print(llvm::raw_ostream &OS) const;
};

} // end namespace index
} // end namespace latino

#endif // LLVM_LATINO_LIB_INDEX_FILEINDEXRECORD_H
