//===--- PragmaKinds.h - #pragma comment() kinds  ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_BASIC_PRAGMA_KINDS_H
#define LLVM_LATINO_BASIC_PRAGMA_KINDS_H

namespace latino {

enum PragmaMSCommentKind {
  PCK_Unknown,
  PCK_Linker,   // #pragma comment(linker, ...)
  PCK_Lib,      // #pragma comment(lib, ...)
  PCK_Compiler, // #pragma comment(compiler, ...)
  PCK_ExeStr,   // #pragma comment(exestr, ...)
  PCK_User      // #pragma comment(user, ...)
};

enum PragmaMSStructKind {
  PMSST_OFF, // #pragms ms_struct off
  PMSST_ON   // #pragms ms_struct on
};

enum PragmaFloatControlKind {
  PFC_Unknown,
  PFC_Precise,   // #pragma float_control(precise, [,on])
  PFC_NoPrecise, // #pragma float_control(precise, off)
  PFC_Except,    // #pragma float_control(except [,on])
  PFC_NoExcept,  // #pragma float_control(except, off)
  PFC_Push,      // #pragma float_control(push)
  PFC_Pop        // #pragma float_control(pop)
};
}

#endif
