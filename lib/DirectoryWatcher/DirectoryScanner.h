//===- DirectoryScanner.h - Utility functions for DirectoryWatcher --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "latino/DirectoryWatcher/DirectoryWatcher.h"
#include "llvm/Support/FileSystem.h"
#include <string>
#include <vector>

namespace latino {

/// Gets names (filenames) of items in directory at \p Path.
/// \returns empty vector if \p Path is not a directory, doesn't exist or can't
/// be read from.
std::vector<std::string> scanDirectory(llvm::StringRef Path);

/// Create event with EventKind::Added for every element in \p Scan.
std::vector<DirectoryWatcher::Event>
getAsFileEvents(const std::vector<std::string> &Scan);

/// Gets status of file (or directory) at \p Path.
/// \returns llvm::None if \p Path doesn't exist or can't get the status.
llvm::Optional<llvm::sys::fs::file_status> getFileStatus(llvm::StringRef Path);

} // namespace latino
