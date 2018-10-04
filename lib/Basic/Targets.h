//===----------------------------------------------------------------------===//
//
// This file declares things required for construction of a TargetInfo object
// from a target triple. Typically individual targets will need to include from
// here in order to get these functions if required.
//
//===----------------------------------------------------------------------===//

#ifndef LATINO_LIB_BASIC_TARGETS_H
#define LATINO_LIB_BASIC_TARGETS_H

#include "latino/Basic/LangOptions.h"
#include "latino/Basic/TargetInfo.h"
#include "llvm/ADT/StringRef.h"

namespace latino {
namespace targets {
    LLVM_LIBRARY_VISIBILITY
    latino::TargetInfo *AllocateTarget(const llvm::Triple &Triple,
                                       const latino::TargetOptions &Opts);

} /* namespace targets */
} /* namespace latino */

#endif
