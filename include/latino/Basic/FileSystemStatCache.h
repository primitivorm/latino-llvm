//===----------------------------------------------------------------------===//
//
/// \file
/// Defines the FileSystemStatCache interface.
//
//===----------------------------------------------------------------------===//

#ifndef LATINO_BASIC_FILESYSTEMSTATCACHE_H
#define LATINO_BASIC_FILESYSTEMSTATCACHE_H

#include "latino/Basic/LLVM.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/FileSystem.h"
#include <cstdint>
#include <ctime>
#include <memory>
#include <string>
#include <utility>

namespace latino {
namespace vfs {
class File;
class FileSystem;
} /* namespace vfs */

// FIXME: should probably replace this with vfs::Status
struct FileData {
  std::string Name;
  uint64_t Size = 0;
  time_t ModTime = 0;
  llvm::sys::fs::UniqueID UniqueID;
  bool IsDirectory = false;
  bool IsNamedPipe = false;
  bool InPCH = false;

  // FIXME: remove this when files support multiple names
  bool IsVFSMapped = false;

  FileData() = default;
}; /* struct FileData */

/// Abstract interface for introducing a FileManager cache for 'stat'
/// system calls, which is used by precompiled and pretokenized headers to
/// improve performance.
class FileSystemStatCache {
  virtual void anchor();

protected:
  std::unique_ptr<FileSystemStatCache> NextStarCache;

public:
  virtual ~FileSystemStatCache() = default;

  enum LookupResult {
    /// We know the file exists and its cached stat data.
    CacheExists,

    /// We know that the file doesn't exist.
    CacheMissing
  }; /* enum LookupResult */

  /// Get the 'stat' information for the specified path, using the cache
  /// to accelerate it if possible.
  ///
  /// \returns \c true if the path does not exist or \c false if it exists.
  ///
  /// If isFile is true, then this lookup should only return success for files
  /// (not directories).  If it is false this lookup should only return
  /// success for directories (not files).  On a successful file lookup, the
  /// implementation can optionally fill in \p F with a valid \p File object and
  /// the client guarantees that it will close it.
  static bool get(StringRef Path, FileData &Data, bool isFile,
                  std::unique_ptr<vfs::File> *F, FileSystemStatCache *Cache,
                  vfs::FileSystem &FS);

}; /* class FileSystemStatCache */

} /* namespace latino */

#endif /* LATINO_BASIC_FILESYSTEMSTATCACHE_H */
