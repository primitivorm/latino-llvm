#include "latino/Basic/IdentifierTable.h"
#include "latino/Basic/CharInfo.h"
#include "latino/Basic/LangOptions.h"
#include "latino/Basic/OperatorKinds.h"
#include "latino/Basic/Specifiers.h"
#include "latino/Basic/TokenKinds.h"
#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/ADT/FoldingSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_os_ostream.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>

using namespace latino;

//===----------------------------------------------------------------------===//
// IdentifierInfo Implementation
//===----------------------------------------------------------------------===//

