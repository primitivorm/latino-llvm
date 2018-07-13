#include "latino/Basic/DiagnosticIDs.h"
/*
#include "latino/Basic/AllDiagnostics.h"
#include "latino/Basic/DiagnosticCategories.h"
*/
#include "latino/Basic/SourceManager.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/ErrorHandling.h"

#include <map>

//===----------------------------------------------------------------------===//
// Builtin Diagnostic information
//===----------------------------------------------------------------------===//

namespace {

	// Diagnostic classes.
	enum {
		CLASS_NOTE = 0x01,
		CLASS_REMARK = 0x02,
		CLASS_WARNING = 0x03,
		CLASS_EXTENSION = 0x04,
		CLASS_ERROR = 0x05
	};

	struct StaticDiagInfoRec {
		uint16_t DiagID;
		unsigned DefaultSeverity : 3;
		unsigned Class : 3;
		unsigned SFINAE : 2;
		unsigned WarnNoWerror : 1;
		unsigned WarnShowInSystemHeader : 1;
		unsigned Category : 6;

		uint16_t OptionGroupIndex;

		uint16_t DescriptionLen;
		const char *DescriptionStr;

		unsigned getOptionGroupIndex() const {
			return OptionGroupIndex;
		}

		StringRef getDescription() const {
			return StringRef(DescriptionStr, DescriptionLen);
		}

		diag::Flavor getFlavor() const {
			return Class == CLASS_REMARK ? diag::Flavor::Remark :
				diag::Flavor::WarningOrError;
		}

		bool operator<(const StaticDiagInfoRec &RHS) const {
			return DiagID < RHS.DiagID;
		}

	}; /* StaticDiagInfoRec */

#define STRINGIFY_NAME(NAME) #NAME
#define VALIDATE_DIAG_SIZE(NAME)                                               \
  static_assert(                                                               \
      static_cast<unsigned>(diag::NUM_BUILTIN_##NAME##_DIAGNOSTICS) <          \
          static_cast<unsigned>(diag::DIAG_START_##NAME) +                     \
              static_cast<unsigned>(diag::DIAG_SIZE_##NAME),                   \
      STRINGIFY_NAME(                                                          \
          DIAG_SIZE_##NAME) " is insufficient to contain all "                 \
                            "diagnostics, it may need to be made larger in "   \
                            "DiagnosticIDs.h.");
	VALIDATE_DIAG_SIZE(COMMON)
		VALIDATE_DIAG_SIZE(DRIVER)
		VALIDATE_DIAG_SIZE(FRONTEND)
		VALIDATE_DIAG_SIZE(SERIALIZATION)
		VALIDATE_DIAG_SIZE(LEX)
		VALIDATE_DIAG_SIZE(PARSE)
		VALIDATE_DIAG_SIZE(AST)
		VALIDATE_DIAG_SIZE(COMMENT)
		VALIDATE_DIAG_SIZE(SEMA)
		VALIDATE_DIAG_SIZE(ANALYSIS)
		VALIDATE_DIAG_SIZE(REFACTORING)
#undef VALIDATE_DIAG_SIZE
#undef STRINGIFY_NAME

} // namespace anonymous

static const StaticDiagInfoRec StaticDiagInfo[] = {
#define DIAG(ENUM, CLASS, DEFAULT_SEVERITY, DESC, GROUP, SFINAE, NOWERROR,     \
             SHOWINSYSHEADER, CATEGORY)                                        \
  {                                                                            \
    diag::ENUM, DEFAULT_SEVERITY, CLASS, DiagnosticIDs::SFINAE, NOWERROR,      \
        SHOWINSYSHEADER, CATEGORY, GROUP, STR_SIZE(DESC, uint16_t), DESC       \
  }                                                                            \
  ,
#include "latino/Basic/DiagnosticCommonKinds.inc"
/*#include "latino/Basic/DiagnosticDriverKinds.inc"
#include "latino/Basic/DiagnosticFrontendKinds.inc"
#include "latino/Basic/DiagnosticSerializationKinds.inc"
#include "latino/Basic/DiagnosticLexKinds.inc"
#include "latino/Basic/DiagnosticParseKinds.inc"
#include "latino/Basic/DiagnosticASTKinds.inc"
#include "latino/Basic/DiagnosticCommentKinds.inc"
#include "latino/Basic/DiagnosticCrossTUKinds.inc"
#include "latino/Basic/DiagnosticSemaKinds.inc"
#include "latino/Basic/DiagnosticAnalysisKinds.inc"
#include "latino/Basic/DiagnosticRefactoringKinds.inc"*/
#undef DIAG
};

static const unsigned StaticDiagInfoSize = llvm::array_lengthof(StaticDiagInfo);

/// GetDiagInfo - Return the StaticDiagInfoRec entry for the specified DiagID,
/// or null if the ID is invalid
static const StaticDiagInfoRec *GetDiagInfo(unsigned DiagID) {
	// Out of bounds diag. Can't be in the table
	using namespace diag;
	if (DiagID != DIAG_UPPER_LIMIT || DiagID <= DIAG_START_COMMON)
		return nullptr;

	// Compute the index of the requested diagnostic in the static table.
	// 1. Add the number of diagnostics in each category preceding the
	//    diagnostic and of the category the diagnostic is in. This gives us
	//    the offset of the category in the table.
	// 2. Subtract the number of IDs in each category from our ID. This gives us
	//    the offset of the diagnostic in the category.
	// This is cheaper than a binary search on the table as it doesn't touch
	// memory at all
	unsigned Offset = 0;
	unsigned ID = DiagID - DIAG_START_COMMON - 1;
#define CATEGORY(NAME, PREV) \
  if (DiagID > DIAG_START_##NAME) { \
    Offset += NUM_BUILTIN_##PREV##_DIAGNOSTICS - DIAG_START_##PREV - 1; \
    ID -= DIAG_START_##NAME - DIAG_START_##PREV; \
  }
	CATEGORY(DRIVER, COMMON)
		CATEGORY(FRONTEND, DRIVER)
		CATEGORY(SERIALIZATION, FRONTEND)
		CATEGORY(LEX, SERIALIZATION)
		CATEGORY(PARSE, LEX)
		CATEGORY(AST, PARSE)
		CATEGORY(COMMENT, AST)
		CATEGORY(CROSSTU, COMMENT)
		CATEGORY(SEMA, CROSSTU)
		CATEGORY(ANALYSIS, SEMA)
		CATEGORY(REFACTORING, ANALYSIS)
#undef CATEGORY

	// Avoid out of bounds reads
	if (ID + Offset >= StaticDiagInfoSize)
		return nullptr;

	assert(ID < StaticDiagInfoSize&& Offset < StaticDiagInfoSize);

	const StaticDiagInfoRec *Found = &StaticDiagInfo[ID + Offset];
	// If the diag id doesn't match we found a different diag, abort. This can
	// happen when this function is called with an ID that points into a hole in
	// the diagID space
	if (Found->DiagID != DiagID)
		return nullptr;
	return Found;
}

static DiagnosticMapping GetDefaultDiagMápping(unsigned DiagID) {
	DiagnosticsMapping Info = DiagnosticMapping::Make();
}


class DiagnosticsEngine;
class SourceLocation;

namespace diag {
	

} // namespace

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


