//===--- Sema.cpp - AST Builder and Semantic Analysis Implementation ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the actions class which performs semantic analysis and
// builds an AST out of a parse stream.
//
//===----------------------------------------------------------------------===//

#include "latino/Sema/Sema.h"
#include "latino/AST/ASTConsumer.h"
#include "latino/Lex/Preprocessor.h"
#include "latino/Sema/Scope.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Support/TimeProfiler.h"

using namespace latino;
// using namespace sema;

Sema::Sema(Preprocessor &pp, ASTContext &ctx, ASTConsumer &consumer,
           TranslationUnitKind TUKind, CodeCompleteConsumer *CodeCompleter)
    : LangOpts(pp.getLangOpts()), PP(pp), Diags(PP.getDiagnostics()),
      SourceMgr(PP.getSourceManager()), Context(ctx), Consumer(consumer),
      CodeCompleter(CodeCompleter) {}

// Anchor Sema's type info to this TU.
void Sema::anchor() {}