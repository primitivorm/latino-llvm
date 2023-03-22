//===--- CapturedStmt.h - Types for CapturedStmts ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_LATINO_BASIC_CAPTUREDSTMT_H
#define LLVM_LATINO_BASIC_CAPTUREDSTMT_H

namespace latino {

/// The different kinds of captured statement.
enum CapturedRegionKind {
  CR_Default,
  CR_ObjCAtFinally,
  CR_OpenMP
};

} // end namespace latino

#endif // LLVM_LATINO_BASIC_CAPTUREDSTMT_H
