//== ProgramState_Fwd.h - Incomplete declarations of ProgramState -*- C++ -*--=/
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_STATICANALYZER_CORE_PATHSENSITIVE_PROGRAMSTATE_FWD_H
#define LLVM_LATINO_STATICANALYZER_CORE_PATHSENSITIVE_PROGRAMSTATE_FWD_H

#include "latino/Basic/LLVM.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"

namespace latino {
namespace ento {
  class ProgramState;
  class ProgramStateManager;
  void ProgramStateRetain(const ProgramState *state);
  void ProgramStateRelease(const ProgramState *state);
}
}

namespace llvm {
  template <> struct IntrusiveRefCntPtrInfo<const latino::ento::ProgramState> {
    static void retain(const latino::ento::ProgramState *state) {
      latino::ento::ProgramStateRetain(state);
    }
    static void release(const latino::ento::ProgramState *state) {
      latino::ento::ProgramStateRelease(state);
    }
  };
}

namespace latino {
namespace ento {
  typedef IntrusiveRefCntPtr<const ProgramState> ProgramStateRef;
}
}

#endif

