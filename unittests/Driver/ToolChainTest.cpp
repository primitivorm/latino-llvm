//===- unittests/Driver/ToolChainTest.cpp --- ToolChain tests -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Unit tests for ToolChains.
//
//===----------------------------------------------------------------------===//

#include "latino/Driver/ToolChain.h"
#include "latino/Basic/DiagnosticIDs.h"
#include "latino/Basic/DiagnosticOptions.h"
#include "latino/Basic/LLVM.h"
#include "latino/Driver/Compilation.h"
#include "latino/Driver/Driver.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/VirtualFileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "gtest/gtest.h"
using namespace latino;
using namespace latino::driver;

namespace {

TEST(ToolChainTest, VFSGCCInstallation) {
  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();

  IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
  struct TestDiagnosticConsumer : public DiagnosticConsumer {};
  DiagnosticsEngine Diags(DiagID, &*DiagOpts, new TestDiagnosticConsumer);
  IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> InMemoryFileSystem(
      new llvm::vfs::InMemoryFileSystem);
  Driver TheDriver("/bin/clang", "arm-linux-gnueabihf", Diags,
                   InMemoryFileSystem);

  const char *EmptyFiles[] = {
      "foo.cpp",
      "/bin/clang",
      "/usr/lib/gcc/arm-linux-gnueabi/4.6.1/crtbegin.o",
      "/usr/lib/gcc/arm-linux-gnueabi/4.6.1/crtend.o",
      "/usr/lib/gcc/arm-linux-gnueabihf/4.6.3/crtbegin.o",
      "/usr/lib/gcc/arm-linux-gnueabihf/4.6.3/crtend.o",
      "/usr/lib/arm-linux-gnueabi/crt1.o",
      "/usr/lib/arm-linux-gnueabi/crti.o",
      "/usr/lib/arm-linux-gnueabi/crtn.o",
      "/usr/lib/arm-linux-gnueabihf/crt1.o",
      "/usr/lib/arm-linux-gnueabihf/crti.o",
      "/usr/lib/arm-linux-gnueabihf/crtn.o",
      "/usr/include/arm-linux-gnueabi/.keep",
      "/usr/include/arm-linux-gnueabihf/.keep",
      "/lib/arm-linux-gnueabi/.keep",
      "/lib/arm-linux-gnueabihf/.keep"};

  for (const char *Path : EmptyFiles)
    InMemoryFileSystem->addFile(Path, 0,
                                llvm::MemoryBuffer::getMemBuffer("\n"));

  std::unique_ptr<Compilation> C(TheDriver.BuildCompilation(
      {"-fsyntax-only", "--gcc-toolchain=", "--sysroot=", "foo.cpp"}));
  EXPECT_TRUE(C);

  std::string S;
  {
    llvm::raw_string_ostream OS(S);
    C->getDefaultToolChain().printVerboseInfo(OS);
  }
#if _WIN32
  std::replace(S.begin(), S.end(), '\\', '/');
#endif
  EXPECT_EQ(
      "Found candidate GCC installation: "
      "/usr/lib/gcc/arm-linux-gnueabihf/4.6.3\n"
      "Selected GCC installation: /usr/lib/gcc/arm-linux-gnueabihf/4.6.3\n"
      "Candidate multilib: .;@m32\n"
      "Selected multilib: .;@m32\n",
      S);
}

TEST(ToolChainTest, VFSGCCInstallationRelativeDir) {
  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();

  IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
  struct TestDiagnosticConsumer : public DiagnosticConsumer {};
  DiagnosticsEngine Diags(DiagID, &*DiagOpts, new TestDiagnosticConsumer);
  IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> InMemoryFileSystem(
      new llvm::vfs::InMemoryFileSystem);
  Driver TheDriver("/home/test/bin/clang", "arm-linux-gnueabi", Diags,
                   InMemoryFileSystem);

  const char *EmptyFiles[] = {
      "foo.cpp", "/home/test/lib/gcc/arm-linux-gnueabi/4.6.1/crtbegin.o",
      "/home/test/include/arm-linux-gnueabi/.keep"};

  for (const char *Path : EmptyFiles)
    InMemoryFileSystem->addFile(Path, 0,
                                llvm::MemoryBuffer::getMemBuffer("\n"));

  std::unique_ptr<Compilation> C(TheDriver.BuildCompilation(
      {"-fsyntax-only", "--gcc-toolchain=", "foo.cpp"}));
  EXPECT_TRUE(C);

  std::string S;
  {
    llvm::raw_string_ostream OS(S);
    C->getDefaultToolChain().printVerboseInfo(OS);
  }
#if _WIN32
  std::replace(S.begin(), S.end(), '\\', '/');
#endif
  EXPECT_EQ("Found candidate GCC installation: "
            "/home/test/bin/../lib/gcc/arm-linux-gnueabi/4.6.1\n"
            "Selected GCC installation: "
            "/home/test/bin/../lib/gcc/arm-linux-gnueabi/4.6.1\n"
            "Candidate multilib: .;@m32\n"
            "Selected multilib: .;@m32\n",
            S);
}

TEST(ToolChainTest, DefaultDriverMode) {
  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();

  IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
  struct TestDiagnosticConsumer : public DiagnosticConsumer {};
  DiagnosticsEngine Diags(DiagID, &*DiagOpts, new TestDiagnosticConsumer);
  IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> InMemoryFileSystem(
      new llvm::vfs::InMemoryFileSystem);

  Driver CCDriver("/home/test/bin/clang", "arm-linux-gnueabi", Diags,
                  InMemoryFileSystem);
  CCDriver.setCheckInputsExist(false);
  Driver CXXDriver("/home/test/bin/clang++", "arm-linux-gnueabi", Diags,
                   InMemoryFileSystem);
  CXXDriver.setCheckInputsExist(false);
  Driver CLDriver("/home/test/bin/clang-cl", "arm-linux-gnueabi", Diags,
                  InMemoryFileSystem);
  CLDriver.setCheckInputsExist(false);

  std::unique_ptr<Compilation> CC(CCDriver.BuildCompilation(
      { "/home/test/bin/clang", "foo.cpp"}));
  std::unique_ptr<Compilation> CXX(CXXDriver.BuildCompilation(
      { "/home/test/bin/clang++", "foo.cpp"}));
  std::unique_ptr<Compilation> CL(CLDriver.BuildCompilation(
      { "/home/test/bin/clang-cl", "foo.cpp"}));

  EXPECT_TRUE(CC);
  EXPECT_TRUE(CXX);
  EXPECT_TRUE(CL);
  EXPECT_TRUE(CCDriver.CCCIsCC());
  EXPECT_TRUE(CXXDriver.CCCIsCXX());
  EXPECT_TRUE(CLDriver.IsCLMode());
}
TEST(ToolChainTest, InvalidArgument) {
  IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
  struct TestDiagnosticConsumer : public DiagnosticConsumer {};
  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
  DiagnosticsEngine Diags(DiagID, &*DiagOpts, new TestDiagnosticConsumer);
  Driver TheDriver("/bin/clang", "arm-linux-gnueabihf", Diags);
  std::unique_ptr<Compilation> C(TheDriver.BuildCompilation(
      {"-fsyntax-only", "-fan-unknown-option", "foo.cpp"}));
  EXPECT_TRUE(C);
  EXPECT_TRUE(C->containsError());
}

TEST(ToolChainTest, ParsedClangName) {
  ParsedClangName Empty;
  EXPECT_TRUE(Empty.TargetPrefix.empty());
  EXPECT_TRUE(Empty.ModeSuffix.empty());
  EXPECT_TRUE(Empty.DriverMode == nullptr);
  EXPECT_FALSE(Empty.TargetIsValid);

  ParsedClangName DriverOnly("clang", nullptr);
  EXPECT_TRUE(DriverOnly.TargetPrefix.empty());
  EXPECT_TRUE(DriverOnly.ModeSuffix == "clang");
  EXPECT_TRUE(DriverOnly.DriverMode == nullptr);
  EXPECT_FALSE(DriverOnly.TargetIsValid);

  ParsedClangName DriverOnly2("clang++", "--driver-mode=g++");
  EXPECT_TRUE(DriverOnly2.TargetPrefix.empty());
  EXPECT_TRUE(DriverOnly2.ModeSuffix == "clang++");
  EXPECT_STREQ(DriverOnly2.DriverMode, "--driver-mode=g++");
  EXPECT_FALSE(DriverOnly2.TargetIsValid);

  ParsedClangName TargetAndMode("i386", "clang-g++", "--driver-mode=g++", true);
  EXPECT_TRUE(TargetAndMode.TargetPrefix == "i386");
  EXPECT_TRUE(TargetAndMode.ModeSuffix == "clang-g++");
  EXPECT_STREQ(TargetAndMode.DriverMode, "--driver-mode=g++");
  EXPECT_TRUE(TargetAndMode.TargetIsValid);
}

TEST(ToolChainTest, GetTargetAndMode) {
  llvm::InitializeAllTargets();
  std::string IgnoredError;
  if (!llvm::TargetRegistry::lookupTarget("x86_64", IgnoredError))
    return;

  ParsedClangName Res = ToolChain::getTargetAndModeFromProgramName("clang");
  EXPECT_TRUE(Res.TargetPrefix.empty());
  EXPECT_TRUE(Res.ModeSuffix == "clang");
  EXPECT_TRUE(Res.DriverMode == nullptr);
  EXPECT_FALSE(Res.TargetIsValid);

  Res = ToolChain::getTargetAndModeFromProgramName("clang++");
  EXPECT_TRUE(Res.TargetPrefix.empty());
  EXPECT_TRUE(Res.ModeSuffix == "clang++");
  EXPECT_STREQ(Res.DriverMode, "--driver-mode=g++");
  EXPECT_FALSE(Res.TargetIsValid);

  Res = ToolChain::getTargetAndModeFromProgramName("clang++6.0");
  EXPECT_TRUE(Res.TargetPrefix.empty());
  EXPECT_TRUE(Res.ModeSuffix == "clang++");
  EXPECT_STREQ(Res.DriverMode, "--driver-mode=g++");
  EXPECT_FALSE(Res.TargetIsValid);

  Res = ToolChain::getTargetAndModeFromProgramName("clang++-release");
  EXPECT_TRUE(Res.TargetPrefix.empty());
  EXPECT_TRUE(Res.ModeSuffix == "clang++");
  EXPECT_STREQ(Res.DriverMode, "--driver-mode=g++");
  EXPECT_FALSE(Res.TargetIsValid);

  Res = ToolChain::getTargetAndModeFromProgramName("x86_64-clang++");
  EXPECT_TRUE(Res.TargetPrefix == "x86_64");
  EXPECT_TRUE(Res.ModeSuffix == "clang++");
  EXPECT_STREQ(Res.DriverMode, "--driver-mode=g++");
  EXPECT_TRUE(Res.TargetIsValid);

  Res = ToolChain::getTargetAndModeFromProgramName(
      "x86_64-linux-gnu-clang-c++");
  EXPECT_TRUE(Res.TargetPrefix == "x86_64-linux-gnu");
  EXPECT_TRUE(Res.ModeSuffix == "clang-c++");
  EXPECT_STREQ(Res.DriverMode, "--driver-mode=g++");
  EXPECT_TRUE(Res.TargetIsValid);

  Res = ToolChain::getTargetAndModeFromProgramName(
      "x86_64-linux-gnu-clang-c++-tot");
  EXPECT_TRUE(Res.TargetPrefix == "x86_64-linux-gnu");
  EXPECT_TRUE(Res.ModeSuffix == "clang-c++");
  EXPECT_STREQ(Res.DriverMode, "--driver-mode=g++");
  EXPECT_TRUE(Res.TargetIsValid);

  Res = ToolChain::getTargetAndModeFromProgramName("qqq");
  EXPECT_TRUE(Res.TargetPrefix.empty());
  EXPECT_TRUE(Res.ModeSuffix.empty());
  EXPECT_TRUE(Res.DriverMode == nullptr);
  EXPECT_FALSE(Res.TargetIsValid);

  Res = ToolChain::getTargetAndModeFromProgramName("x86_64-qqq");
  EXPECT_TRUE(Res.TargetPrefix.empty());
  EXPECT_TRUE(Res.ModeSuffix.empty());
  EXPECT_TRUE(Res.DriverMode == nullptr);
  EXPECT_FALSE(Res.TargetIsValid);

  Res = ToolChain::getTargetAndModeFromProgramName("qqq-clang-cl");
  EXPECT_TRUE(Res.TargetPrefix == "qqq");
  EXPECT_TRUE(Res.ModeSuffix == "clang-cl");
  EXPECT_STREQ(Res.DriverMode, "--driver-mode=cl");
  EXPECT_FALSE(Res.TargetIsValid);
}
} // end anonymous namespace.
