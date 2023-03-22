//== SMTConstraintManager.cpp -----------------------------------*- C++ -*--==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "latino/StaticAnalyzer/Core/PathSensitive/SMTConstraintManager.h"

using namespace latino;
using namespace ento;

std::unique_ptr<ConstraintManager>
ento::CreateZ3ConstraintManager(ProgramStateManager &StMgr, ExprEngine *Eng) {
  return std::make_unique<SMTConstraintManager>(Eng, StMgr.getSValBuilder());
}
