//===--- SanitizerSpecialCaseList.cpp - SCL for sanitizers ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// An extension of SpecialCaseList to allowing querying sections by
// SanitizerMask.
//
//===----------------------------------------------------------------------===//
#include "latino/Basic/SanitizerSpecialCaseList.h"

using namespace latino;

std::unique_ptr<SanitizerSpecialCaseList>
SanitizerSpecialCaseList::create(const std::vector<std::string> &Paths,
                                 llvm::vfs::FileSystem &VFS,
                                 std::string &Error) {
  std::unique_ptr<latino::SanitizerSpecialCaseList> SSCL(
      new SanitizerSpecialCaseList());
  if (SSCL->createInternal(Paths, VFS, Error)) {
    SSCL->createSanitizerSections();
    return SSCL;
  }
  return nullptr;
}

std::unique_ptr<SanitizerSpecialCaseList>
SanitizerSpecialCaseList::createOrDie(const std::vector<std::string> &Paths,
                                      llvm::vfs::FileSystem &VFS) {
  std::string Error;
  if (auto SSCL = create(Paths, VFS, Error))
    return SSCL;
  llvm::report_fatal_error(Error);
}

void SanitizerSpecialCaseList::createSanitizerSections() {
  for (auto &S : Sections) {
    SanitizerMask Mask;

#define SANITIZER(NAME, ID)                                                    \
  if (S.SectionMatcher->match(NAME))                                           \
    Mask |= SanitizerKind::ID;
#define SANITIZER_GROUP(NAME, ID, ALIAS) SANITIZER(NAME, ID)

#include "latino/Basic/Sanitizers.def"
#undef SANITIZER
#undef SANITIZER_GROUP

    SanitizerSections.emplace_back(Mask, S.Entries);
  }
}

bool SanitizerSpecialCaseList::inSection(SanitizerMask Mask, StringRef Prefix,
                                         StringRef Query,
                                         StringRef Category) const {
  for (auto &S : SanitizerSections)
    if ((S.Mask & Mask) &&
        SpecialCaseList::inSectionBlame(S.Entries, Prefix, Query, Category))
      return true;

  return false;
}
