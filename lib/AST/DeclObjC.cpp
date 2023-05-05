//===- DeclObjC.cpp - ObjC Declaration AST Node Implementation ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the Objective-C related Decl classes.
//
//===----------------------------------------------------------------------===//

#include "latino/AST/DeclObjC.h"
#include "latino/AST/ASTContext.h"
#include "latino/AST/ASTMutationListener.h"
#include "latino/AST/Attr.h"
#include "latino/AST/Decl.h"
#include "latino/AST/DeclBase.h"
#include "latino/AST/Stmt.h"
#include "latino/AST/Type.h"
#include "latino/AST/TypeLoc.h"
#include "latino/Basic/IdentifierTable.h"
#include "latino/Basic/LLVM.h"
#include "latino/Basic/LangOptions.h"
#include "latino/Basic/SourceLocation.h"
#include "llvm/ADT/None.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <utility>

using namespace latino;

//===----------------------------------------------------------------------===//
// ObjCIvarDecl
//===----------------------------------------------------------------------===//

void ObjCIvarDecl::anchor() {}