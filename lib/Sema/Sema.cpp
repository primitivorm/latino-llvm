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
#include "latino/Sema/CodeCompleteConsumer.h"
#include "latino/AST/ASTContext.h"
#include "latino/AST/Decl.h"
#include "latino/AST/Stmt.h"
#include "latino/AST/ASTConsumer.h"
#include "latino/Parse/Parser.h"
#include "latino/Lex/Preprocessor.h"
#include "clang/Basic/LangOptions.h"
#include "llvm/Support/CrashRecoveryContext.h"

using namespace latino;
using namespace sema;

latino::Sema::Sema(latino::Preprocessor &pp, latino::ASTContext &ctxt, latino::ASTConsumer &consumer,
           clang::TranslationUnitKind TUKind, latino::CodeCompleteConsumer *CodeCompleter): 
    PP(pp), Context(ctxt), Consumer(consumer), TUKind(TUKind),
      CodeCompleter(CodeCompleter), CollectStats(false) {

           }