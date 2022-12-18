//===--- GlobalModuleIndex.cpp - Global Module Index ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the GlobalModuleIndex class.
//
//===----------------------------------------------------------------------===//
#include "latino/Serialization/GlobalModuleIndex.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Bitstream/BitstreamReader.h"
#include "llvm/Bitstream/BitstreamWriter.h"
#include "llvm/Support/DJB.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/FileUtilities.h"
#include "llvm/Support/LockFileManager.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/OnDiskHashTable.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/TimeProfiler.h"

#include <cstdio>

using namespace latino;
// using namespace serialization;

GlobalModuleIndex::GlobalModuleIndex(
    std::unique_ptr<llvm::MemoryBuffer> IndexBuffer,
    llvm::BitstreamCursor Cursor)
    : Buffer(std::move(IndexBuffer)), IdentifierIndex(), NumIdentifierLookups(),
      NumIdentifierLookupHits() {}