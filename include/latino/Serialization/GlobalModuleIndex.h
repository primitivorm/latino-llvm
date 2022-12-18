//===--- GlobalModuleIndex.h - Global Module Index --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the GlobalModuleIndex class, which manages a global index
// containing all of the identifiers known to the various modules within a given
// subdirectory of the module cache. It is used to improve the performance of
// queries such as "do any modules know about this identifier?"
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LATINO_SERIALIZATION_GLOBALMODULEINDEX_H
#define LLVM_LATINO_SERIALIZATION_GLOBALMODULEINDEX_H

#include "latino/Basic/FileManager.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include <memory>
#include <utility>

namespace llvm {
class BitstreamCursor;
class MemoryBuffer;
} // namespace llvm

namespace latino {
class FileManager;
class PCHContainerReader;

/// A global index for a set of module files, providing information about
/// the identifiers within those module files.
///
/// The global index is an aid for name lookup into modules, offering a central
/// place where one can look for identifiers determine which
/// module files contain any information about that identifier. This
/// allows the client to restrict the search to only those module files known
/// to have a information about that identifier, improving performance.
/// Moreover, the global module index may know about module files that have not
/// been imported, and can be queried to determine which modules the current
/// translation could or should load to fix a problem.
class GlobalModuleIndex {
  /// Buffer containing the index file, which is lazily accessed so long
  /// as the global module index is live.
  std::unique_ptr<llvm::MemoryBuffer> Buffer;

  /// The hash table.
  ///
  /// This pointer actually points to a IdentifierIndexTable object,
  /// but that type is only accessible within the implementation of
  /// GlobalModuleIndex.
  void *IdentifierIndex;

  /// The number of identifier lookups we performed.
  unsigned NumIdentifierLookups;

  /// The number of identifier lookup hits, where we recognize the
  /// identifier.
  unsigned NumIdentifierLookupHits;

  /// Internal constructor. Use \c readIndex() to read an index.
  explicit GlobalModuleIndex(std::unique_ptr<llvm::MemoryBuffer> Buffer,
                             llvm::BitstreamCursor Cursor);

  GlobalModuleIndex(const GlobalModuleIndex &) = delete;
  GlobalModuleIndex &operator=(const GlobalModuleIndex &) = delete;

public:
  ~GlobalModuleIndex();

  /// Write a global index into the given
  ///
  /// \param FileMgr The file manager to use to load module files.
  /// \param PCHContainerRdr - The PCHContainerOperations to use for loading and
  /// creating modules.
  /// \param Path The path to the directory containing module files, into
  /// which the global index will be written.
  static llvm::Error writeIndex(FileManager &FileMgr,
                                const PCHContainerReader &PCHContainerRdr,
                                llvm::StringRef Path);
};
} // namespace latino

#endif