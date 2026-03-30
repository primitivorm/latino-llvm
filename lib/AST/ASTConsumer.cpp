//===--- ASTConsumers.cpp - ASTConsumer implementations -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// AST Consumer Implementations.
//
//===----------------------------------------------------------------------===//
#include "latino/AST/ASTConsumer.h"
#include "latino/AST/ASTContext.h"
#include "latino/AST/DeclGroup.h"

#include "llvm/Support/Path.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/raw_ostream.h"

using namespace latino;

//===----------------------------------------------------------------------===//
/// ASTViewer - AST Visualization

namespace {
  class ASTViewer : public ASTConsumer {
    ASTContext *Context;
  public:
    void Initialize(ASTContext &Context) override {
      this->Context = &Context;
    }

    bool HandleTopLevelDecl(DeclGroupRef D) override {
      for (DeclGroupRef::iterator I = D.begin(), E = D.end(); I != E; ++I)
        HandleTopLevelSingleDecl(*I);
      return true;
    }

    void HandleTopLevelSingleDecl(Decl *D);
  };
}
