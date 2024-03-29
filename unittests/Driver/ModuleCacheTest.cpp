//===- unittests/Driver/ModuleCacheTest.cpp -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Unit tests for the LLDB module cache API.
//
//===----------------------------------------------------------------------===//

#include "latino/Driver/Driver.h"
#include "gtest/gtest.h"
using namespace latino;
using namespace latino::driver;

namespace {

TEST(ModuleCacheTest, GetTargetAndMode) {
  SmallString<128> Buf;
  Driver::getDefaultModuleCachePath(Buf);
  StringRef Path = Buf;
  EXPECT_TRUE(Path.find("clang") != Path.npos);
  EXPECT_TRUE(Path.endswith("ModuleCache"));  
}
} // end anonymous namespace.
