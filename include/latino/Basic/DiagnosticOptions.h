#ifndef LATINO_BASIC_DIAGNOSTICOPTIONS_H
#define LATINO_BASIC_DIAGNOSTICOPTIONS_H

#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include <string>
#include <type_traits>
#include <vector>

namespace latino {

/// Specifies which overload candidates to display when overload
/// resolution fails.
enum OverloadsShown : unsigned {
	/// Show all overloads
	Ovl_All,

	/// Show just the "best" overload candidates
	Ovl_Best
};

class DiagnosticOptions : public RefCountedBase<DiagnosticOptions> {
public:
	DiagnosticOptions() {

	}
};
} // namespace latino

#endif
