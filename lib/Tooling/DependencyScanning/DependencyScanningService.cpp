//===- DependencyScanningService.cpp - clang-scan-deps service ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "latino/Tooling/DependencyScanning/DependencyScanningService.h"

using namespace latino;
using namespace tooling;
using namespace dependencies;

DependencyScanningService::DependencyScanningService(
    ScanningMode Mode, ScanningOutputFormat Format, bool ReuseFileManager,
    bool SkipExcludedPPRanges)
    : Mode(Mode), Format(Format), ReuseFileManager(ReuseFileManager),
      SkipExcludedPPRanges(SkipExcludedPPRanges) {}
