#ifndef LATINO_BASIC_DIAGNOSTICOPTIONS_H
#define LATINO_BASIC_DIAGNOSTICOPTIONS_H

#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include <string>
#include <type_traits>
#include <vector>

namespace latino {
class DiagnosticOptions : public RefCountedBase<DiagnosticOptions> {
public:
	DiagnosticOptions() {

	}
};
} // namespace latino

#endif
