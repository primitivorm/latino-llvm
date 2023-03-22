//===- ReachableCode.h -----------------------------------------*- C++ --*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// A flow-sensitive, path-insensitive analysis of unreachable code.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_ANALYSIS_ANALYSES_REACHABLECODE_H
#define LLVM_LATINO_ANALYSIS_ANALYSES_REACHABLECODE_H

#include "latino/Basic/SourceLocation.h"

//===----------------------------------------------------------------------===//
// Forward declarations.
//===----------------------------------------------------------------------===//

namespace llvm {
  class BitVector;
}

namespace latino {
  class AnalysisDeclContext;
  class CFGBlock;
  class Preprocessor;
}

//===----------------------------------------------------------------------===//
// API.
//===----------------------------------------------------------------------===//

namespace latino {
namespace reachable_code {

/// Classifications of unreachable code.
enum UnreachableKind {
  UK_Return,
  UK_Break,
  UK_Loop_Increment,
  UK_Other
};

class Callback {
  virtual void anchor();
public:
  virtual ~Callback() {}
  virtual void HandleUnreachable(UnreachableKind UK,
                                 SourceLocation L,
                                 SourceRange ConditionVal,
                                 SourceRange R1,
                                 SourceRange R2) = 0;
};

/// ScanReachableFromBlock - Mark all blocks reachable from Start.
/// Returns the total number of blocks that were marked reachable.
unsigned ScanReachableFromBlock(const CFGBlock *Start,
                                llvm::BitVector &Reachable);

void FindUnreachableCode(AnalysisDeclContext &AC, Preprocessor &PP,
                         Callback &CB);

}} // end namespace latino::reachable_code

#endif
