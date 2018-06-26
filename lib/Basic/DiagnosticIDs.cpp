#include "latino/Basic/DiagnosticIDs.h"

/*#include "latino/Basic/AllDiagnostics.h"
#include "latino/Basic/DiagnosticCategories.h"
*/

namespace latino {
namespace diag {
class CustomDiagInfo {};
} // namespace diag


DiagnosticIDs::Level
getDiagnosticLevel(unsigned DiagID, SourceLocation Loc,
	const DiagnosticsEngine &Diag) const {
	if (DiagID >= diag::DIAG_UPPER_LIMIT) {
		assert(CustomDiagInfo && "Invalid CustomDiagInfo");
		return CustomDiagInfo->getLevel(DiagID);
	}
	unsigned DiagClass = getBuiltinDiagClass(DiagID);
	if (DiagClass == CLASS_NOTE) return DiagnosticIDs::Note;
	return toLevel(getDiagnosticSeverity(DiagID, Loc, Diag));
}

} // namespace latino
