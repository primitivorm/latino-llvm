//===- LangOptions.cpp - C Language Family Language Options ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the LangOptions class.
//
//===----------------------------------------------------------------------===//

#include "latino/Basic/LangOptions.h"

using namespace latino;

LangOptions::LangOptions() {
#define LANGOPT(Name, Bits, Default, Description) Name = Default;
#define ENUM_LANGOPT(Name, Type, Bits, Default, Description) set##Name(Default);
#include "latino/Basic/LangOptions.def"
}