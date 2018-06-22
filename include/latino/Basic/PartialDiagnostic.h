#ifndef LATINO_BASIC_PARTIAL_DIAGNOSTIC_H
#define LATINO_BASIC_PARTIAL_DIAGNOSTIC_H

#include "latino/Basic/Diagnostic.h"
#include "latino/Basic/LLVM.h"
#include "latino/Basic/SourceLocation.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include <cassert>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>

namespace latino {
class PartialDiagnostic {
private:
  mutable unsigned DiagID = 0;   

public:
  enum { MaxArguments = DiagnosticEngine::MaxArguments };

  struct Storage {
    enum { MaxArguments = PartialDiagnostic::MaxArguments };

	Storage() = default;
  };

  class StorageAllocator {
  public:
    StorageAllocator();
    ~StorageAllocator();
  };

private:
	mutable Storage *DiagStorage = nullptr;
	StorageAllocator *Allocator = nullptr;

	Storage *getStorage() const {
		if (DiagStorage) return DiagStorage;

	}

public:
  struct NullDiagnostic {};
  PartialDiagnostic(NullDiagnostic) {}
  PartialDiagnostic(unsigned DiagID, StorageAllocator &Allocator)
      : DiagID(DiagID), Allocator(&Allocator) {}
  PartialDiagnostic(const PartialDiagnostic &Other)
      : DiagID(Other.DiagID), Allocator(Other.Allocator) {
    if (Other.DiagStorage) {
		DiagStorage = getStorage();
		*DiagStorage = *Other.DiagStorage;
    }
  }
};
using PartialDiagnosticAt = std::pair<SourceLocation, PartialDiagnostic>;

} // namespace latino

#endif
