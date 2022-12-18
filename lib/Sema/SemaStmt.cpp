//===--- SemaStmt.cpp - Semantic Analysis for Statements ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements semantic analysis for statements.
//
//===----------------------------------------------------------------------===//
#include "latino/Basic/SourceManager.h"
#include "latino/Sema/Sema.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"

using namespace latino;

// StmtResult
// latino::Sema::ActOnAttributedStmt(SourceLocation AttrLoc,
//                                   ArrayRef<const clang::Attr *> Attrs,
//                                   Stmt *SubStmt) {
//   // Fill in the declaration and return it.
//   clang::AttributedStmt *LS /*=
//       clang::AttributedStmt::Create(Context, AttrLoc, Attrs, SubStmt)*/
//       ;
//   return LS;
// }