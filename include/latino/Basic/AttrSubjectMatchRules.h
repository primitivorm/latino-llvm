//===-- AttrSubjectMatchRules.h - Attribute subject match rules -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_BASIC_ATTR_SUBJECT_MATCH_RULES_H
#define LLVM_LATINO_BASIC_ATTR_SUBJECT_MATCH_RULES_H

#include "latino/Basic/SourceLocation.h"
#include "llvm/ADT/DenseMap.h"

namespace latino {
namespace attr {

/// A list of all the recognized kinds of attributes.
enum SubjectMatchRule {
#define ATTR_MATCH_RULE(X, Spelling, IsAbstract) X,
#include "latino/Basic/AttrSubMatchRulesList.inc"
};

const char *getSubjectMatchRuleSpelling(SubjectMatchRule Rule);

using ParsedSubjectMatchRuleSet = llvm::DenseMap<int, SourceRange>;

} // end namespace attr
} // end namespace latino

#endif
