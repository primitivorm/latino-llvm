//===--- MigratorOptions.h - MigratorOptions Options ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This header contains the structures necessary for a front-end to specify
// various migration analysis.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_FRONTEND_MIGRATOROPTIONS_H
#define LLVM_LATINO_FRONTEND_MIGRATOROPTIONS_H

namespace latino {

class MigratorOptions {
public:
  unsigned NoNSAllocReallocError : 1;
  unsigned NoFinalizeRemoval : 1;
  MigratorOptions() {
    NoNSAllocReallocError = 0;
    NoFinalizeRemoval = 0;
  }
};

}
#endif
