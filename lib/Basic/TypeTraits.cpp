//===--- TypeTraits.cpp - Type Traits Support -----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements the type traits support functions.
//
//===----------------------------------------------------------------------===//

#include "latino/Basic/TypeTraits.h"
#include "llvm/Support/ErrorHandling.h"
#include <cassert>
using namespace latino;

static constexpr const char *TypeTraitNames[] = {
#define TYPE_TRAIT_1(Spelling, Name, Key) #Name,
#include "latino/Basic/TokenKinds.def"
#define TYPE_TRAIT_2(Spelling, Name, Key) #Name,
#include "latino/Basic/TokenKinds.def"
#define TYPE_TRAIT_N(Spelling, Name, Key) #Name,
#include "latino/Basic/TokenKinds.def"
};

static constexpr const char *TypeTraitSpellings[] = {
#define TYPE_TRAIT_1(Spelling, Name, Key) #Spelling,
#include "latino/Basic/TokenKinds.def"
#define TYPE_TRAIT_2(Spelling, Name, Key) #Spelling,
#include "latino/Basic/TokenKinds.def"
#define TYPE_TRAIT_N(Spelling, Name, Key) #Spelling,
#include "latino/Basic/TokenKinds.def"
};

static constexpr const char *ArrayTypeTraitNames[] = {
#define ARRAY_TYPE_TRAIT(Spelling, Name, Key) #Name,
#include "latino/Basic/TokenKinds.def"
};

static constexpr const char *ArrayTypeTraitSpellings[] = {
#define ARRAY_TYPE_TRAIT(Spelling, Name, Key) #Spelling,
#include "latino/Basic/TokenKinds.def"
};

static constexpr const char *UnaryExprOrTypeTraitNames[] = {
#define UNARY_EXPR_OR_TYPE_TRAIT(Spelling, Name, Key) #Name,
#define CXX11_UNARY_EXPR_OR_TYPE_TRAIT(Spelling, Name, Key) #Name,
#include "latino/Basic/TokenKinds.def"
};

static constexpr const char *UnaryExprOrTypeTraitSpellings[] = {
#define UNARY_EXPR_OR_TYPE_TRAIT(Spelling, Name, Key) #Spelling,
#define CXX11_UNARY_EXPR_OR_TYPE_TRAIT(Spelling, Name, Key) #Spelling,
#include "latino/Basic/TokenKinds.def"
};

const char *latino::getTraitName(TypeTrait T) {
  assert(T <= TT_Last && "invalid enum value!");
  return TypeTraitNames[T];
}

const char *latino::getTraitName(ArrayTypeTrait T) {
  assert(T <= ATT_Last && "invalid enum value!");
  return ArrayTypeTraitNames[T];
}

const char *latino::getTraitName(UnaryExprOrTypeTrait T) {
  assert(T <= UETT_Last && "invalid enum value!");
  return UnaryExprOrTypeTraitNames[T];
}

const char *latino::getTraitSpelling(TypeTrait T) {
  assert(T <= TT_Last && "invalid enum value!");
  return TypeTraitSpellings[T];
}

const char *latino::getTraitSpelling(ArrayTypeTrait T) {
  assert(T <= ATT_Last && "invalid enum value!");
  return ArrayTypeTraitSpellings[T];
}

const char *latino::getTraitSpelling(UnaryExprOrTypeTrait T) {
  assert(T <= UETT_Last && "invalid enum value!");
  return UnaryExprOrTypeTraitSpellings[T];
}
