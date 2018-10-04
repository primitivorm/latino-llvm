#include "latino/Basic/VirtualFileSystem.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/SourceMgr.h"
#include "gtest/gtest.h"
#include <map>
#include <string>

using namespace latino;
using namespace llvm;
using llvm::sys::fs::UniqueID;

namespace {
struct DummyFile : public vfs::File {
  vfs::Status S;
  explicit DummyFile(vfs::Status S) : S(S) {}
  llvm::ErrorOr<vfs::Status> status() override { return S; }
  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>>
  getBuffer(const Twine &Name, int64_t FileSize, bool RequiresNullTerminator,
            bool IsVolatile) override {
    llvm_unreachable("unimplemented");
  }
  std::error_code close() override { return std::error_code(); }
}; // struct DummyFile

class DummyFileSystem : public vfs::FileSystem {
  int FSID;   // used to produce UniqueIDs
  int FileID; // used to produce UniqueIDs
  std::map<std::string, vfs::Status> FilesAndDirs;

  static int getNextFSID() {
    static int Count = 0;
    return Count++;
  }

public:
  DummyFileSystem() : FSID(getNextFSID()), FileID(0) {}

  ErrorOr<vfs::Status> status(const Twine &Path) override {
    std::map<std::string, vfs::Status>::iterator I =
        FilesAndDirs.find(Path.str());
    if (I == FilesAndDirs.end())
      return make_error_code(llvm::errc::no_such_file_or_directory);
    return I->second;
  }
  ErrorOr<std::unique_ptr<vfs::File>>
  openFileForRead(const Twine &Path) override {
    auto S = status(Path);
    if (S)
      return std::unique_ptr<vfs::File>(new DummyFile{*S});
    return S.getError();
  }
  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() const override {
    return std::string();
  }
  std::error_code setCurrentWorkingDirectory(const Twine &Path) override {
    return std::error_code();
  }
  // Map any symlink to "/symlink".
  std::error_code getRealPath(const Twine &Path,
                              SmallVectorImpl<char> &Output) const override {
    auto I = FilesAndDirs.find(Path.str());
    if (I == FilesAndDirs.end())
      return make_error_code(llvm::errc::no_such_file_or_directory);
    if (I->second.isSymlink()) {
      Output.clear();
      Twine("/symlink").toVector(Output);
      return std::error_code();
    }
    Output.clear();
    Path.toVector(Output);
    return std::error_code();
  }

  struct DirIterImpl : public latino::vfs::detail::DirIterImpl {
    std::map<std::string, vfs::Status> &FilesAndDirs;
    std::map<std::string, vfs::Status>::iterator I;
    std::string Path;
    bool isInPath(StringRef S) {
      if (Path.size() < S.size() && S.find(Path) == 0) {
        auto LastSep = S.find_last_of('/');
        if (LastSep == Path.size() || LastSep == Path.size() - 1)
          return true;
      }
      return false;
    }
    DirIterImpl(std::map<std::string, vfs::Status> &FilesAndDirs,
                const Twine &_Path)
        : FilesAndDirs(FilesAndDirs), I(FilesAndDirs.begin()),
          Path(_Path.str()) {
      for (; I != FilesAndDirs.end(); ++I) {
        if (isInPath(I->first)) {
          CurrentEntry = I->second;
          break;
        }
      }
    }
    std::error_code increment() override {
      ++I;
      for (; I != FilesAndDirs.end(); ++I) {
        if (isInPath(I->first)) {
          CurrentEntry = I->second;
          break;
        }
      }
      if (I == FilesAndDirs.end())
        CurrentEntry = vfs::Status();
      return std::error_code();
    }
  }; // struct DirIterImpl

  vfs::directory_iterator dir_begin(const Twine &Dir,
                                    std::error_code &EC) override {
    return vfs::directory_iterator(
        std::make_shared<DirIterImpl>(FilesAndDirs, Dir));
  }

  void addEntry(StringRef Path, const vfs::Status &Status) {
    FilesAndDirs[Path] = Status;
  }

  void addRegularFile(StringRef Path, sys::fs::perms Perms = sys::fs::all_all) {
    vfs::Status S(Path, UniqueID(FSID, FileID++),
                  std::chrono::system_clock::now(), 0, 0, 1024,
                  sys::fs::file_type::regular_file, Perms);
    addEntry(Path, S);
  }

  void addDirectory(StringRef Path, sys::fs::perms Perms = sys::fs::all_all) {
    vfs::Status S(Path, UniqueID(FSID, FileID++),
                  std::chrono::system_clock::now(), 0, 0, 0,
                  sys::fs::file_type::directory_file, Perms);
    addEntry(Path, S);
  }

  void addSymLink(StringRef Path) {
    vfs::Status S(Path, UniqueID(FSID, FileID++),
                  std::chrono::system_clock::now(), 0, 0, 0,
                  sys::fs::file_type::symlink_file, sys::fs::all_all);
    addEntry(Path, S);
  }
}; // class DummyFileSystem

/// Replace back-slashes by front-slashes.
std::string getPosixPath(std::string S) {
  SmallString<128> Result;
  llvm::sys::path::native(S, Result, llvm::sys::path::Style::posix);
  return Result.str();
}

} // namespace

TEST(VirtualFileSystemTest, StatusQueries) {
  IntrusiveRefCntPtr<DummyFileSystem> D(new DummyFileSystem());
  ErrorOr<vfs::Status> Status((std::error_code()));

  D->addRegularFile("/foo");
  Status = D->status("/foo");
  ASSERT_FALSE(Status.getError());
  EXPECT_TRUE(Status->isStatusKnown());
  EXPECT_FALSE(Status->isDirectory());
  EXPECT_TRUE(Status->isRegularFile());
  EXPECT_FALSE(Status->isSymlink());
  EXPECT_FALSE(Status->isOther());
  EXPECT_TRUE(Status->exists());

  D->addDirectory("/bar");
  Status = D->status("/bar");
  ASSERT_FALSE(Status.getError());
  EXPECT_TRUE(Status->isStatusKnown());
  EXPECT_TRUE(Status->isDirectory());
  EXPECT_FALSE(Status->isRegularFile());
  EXPECT_FALSE(Status->isSymlink());
  EXPECT_FALSE(Status->isOther());
  EXPECT_TRUE(Status->exists());

  D->addSymLink("/baz");
  Status = D->status("/baz");
  ASSERT_FALSE(Status.getError());
  EXPECT_TRUE(Status->isStatusKnown());
  EXPECT_FALSE(Status->isDirectory());
  EXPECT_FALSE(Status->isRegularFile());
  EXPECT_TRUE(Status->isSymlink());
  EXPECT_FALSE(Status->isOther());
  EXPECT_TRUE(Status->exists());

  EXPECT_TRUE(Status->equivalent(*Status));
  ErrorOr<vfs::Status> Status2 = D->status("/foo");
  ASSERT_FALSE(Status2.getError());
  EXPECT_FALSE(Status->equivalent(*Status2));
}
