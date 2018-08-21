#include "latino/Basic/DiagnosticOptions.h"
#include "llvm/Support/raw_ostream.h"
#include <type_traits>

namespace latino {
raw_ostream &operator<<(raw_ostream &Out, DiagnosticLevelMask M) {
  using UT = std::underlying_type<DiagnosticLevelMask>::type;
  return Out << static_cast<UT>(M);
}
} /* namespace latino */
