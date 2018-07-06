#ifndef LATINO_BASIC_DIAGNOSTICIDS_H
#define LATINO_BASIC_DIAGNOSTICIDS_H

#include "latino/Basic/LLVM.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/StringRef.h"
#include <vector>

namespace latino {
class DiagnosticsEngine;
class SourceLocation;
    
// Import the diagnostic enums themselves.
namespace diag {
    // Size of each of the diagnostic categories.
    enum {
      DIAG_SIZE_COMMON        =  300,
      DIAG_SIZE_DRIVER        =  200,
      DIAG_SIZE_FRONTEND      =  100,
      DIAG_SIZE_SERIALIZATION =  120,
      DIAG_SIZE_LEX           =  400,
      DIAG_SIZE_PARSE         =  500,
      DIAG_SIZE_AST           =  150,
      DIAG_SIZE_COMMENT       =  100,
      DIAG_SIZE_CROSSTU       =  100,
      DIAG_SIZE_SEMA          = 3500,
      DIAG_SIZE_ANALYSIS      =  100,
      DIAG_SIZE_REFACTORING   = 1000,
    };
    // Start position for diagnostics.
    enum {
      DIAG_START_COMMON        =                          0,
      DIAG_START_DRIVER        = DIAG_START_COMMON        + DIAG_SIZE_COMMON,
      DIAG_START_FRONTEND      = DIAG_START_DRIVER        + DIAG_SIZE_DRIVER,
      DIAG_START_SERIALIZATION = DIAG_START_FRONTEND      + DIAG_SIZE_FRONTEND,
      DIAG_START_LEX           = DIAG_START_SERIALIZATION + DIAG_SIZE_SERIALIZATION,
      DIAG_START_PARSE         = DIAG_START_LEX           + DIAG_SIZE_LEX,
      DIAG_START_AST           = DIAG_START_PARSE         + DIAG_SIZE_PARSE,
      DIAG_START_COMMENT       = DIAG_START_AST           + DIAG_SIZE_AST,
      DIAG_START_CROSSTU       = DIAG_START_COMMENT       + DIAG_SIZE_CROSSTU,
      DIAG_START_SEMA          = DIAG_START_CROSSTU       + DIAG_SIZE_COMMENT,
      DIAG_START_ANALYSIS      = DIAG_START_SEMA          + DIAG_SIZE_SEMA,
      DIAG_START_REFACTORING   = DIAG_START_ANALYSIS      + DIAG_SIZE_ANALYSIS,
      DIAG_UPPER_LIMIT         = DIAG_START_REFACTORING   + DIAG_SIZE_REFACTORING
    };
    
class CustomDiagInfo;

/// All of the diagnostics that can be emitted by the frontend
typedef unsigned kind;

// Get typedefs for common diagnostics.
enum {
#define DIAG(ENUM, FLAGS, DEFAULT_MAPPING, DESC, GROUP, SFINAE, CATEGORY,      \
             NOWERROR, SHOWINSYSHEADER)                                        \
  ENUM,
#define COMMONSTART
#include "latino/Basic/DiagnosticCommonKinds.inc"
  NUM_BUILTIN_COMMON_DIAGNOSTICS
#undef DIAG
};

/// Enum values that allow the client to map NOTEs, WARNINGs, and EXTENSIONs
/// to either Ignore (nothing), Remark (emit a remark), Warning
/// (emit a warning) or Error (emit as an error).  It allows clients to
/// map ERRORs to Error or Fatal (stop emitting diagnostics after this one)
enum class Severity {
	//NOTE: 0 means "uncomputed"
	Ignored = 1, ///< Do not present this diagnostic, ignore it
	Remark = 2,  ///< Present this diagnostic as a remark
	Warning = 3, ///< Present this diagnostic as a warning
	Error = 4,   ///< Present this diagnostic as an error
	Fatal = 5    ///< Present this diagnostic as a fatal error
};

/// Flavors of diagnostics we can emit. Used to filter for a particular
/// kind of diagnostic (for instance, for -W/-R flags)
enum class Flavor {
	WarningOrError,	///< A diagnostic that indicates a problem or potential
					///< problem. Can be made fatal by -Werror
	Remark			///< A diagnostic that indicates normal progress through
					///< compilation
};

} // namespace diag


class DiagnosticMapping {
	unsigned Severity : 3;
	unsigned IsUser : 1;
	unsigned IsPragma : 1;
	unsigned HasNoWarningAsError : 1;
	unsigned HasNoErrorAsFatal : 1;
	unsigned WasUpgradedFromWarning : 1;

public:
	static DiagnosticMapping Make(diag::Severity Severity, bool IsUser, bool IsPragma) {
		DiagnosticMapping Result;
		Result.Severity = (unsigned)Severity;
		Result.IsUser = (unsigned)IsUser;
		Result.IsPragma = (unsigned)IsPragma;
		Result.HasNoWarningAsError = 0;
		Result.HasNoErrorAsFatal = 0;
		Result.WasUpgradedFromWarning = 0;
		return Result;
	}

	diag::Severity getSeverity() const { return (diag::Severity) Severity; }
	void setSeverity(diag::Severity Value) { Severity = (unsigned)Value; }

	bool isUser() const { return IsUser; }
	bool isPragma() const { return IsPragma; }

	bool isErrorOrFatal() const {
		return getSeverity() == diag::Severity::Error ||
			getSeverity() == diag::Severity::Fatal;
	}

	bool hasNoWarningAsError() const { return HasNoWarningAsError; }
	void setNoWarningAsError(bool Value) { HasNoErrorAsFatal = Value; }

	bool hasErrorAsFatal() const { return HasNoErrorAsFatal; }
	void setErrorAsFatal(bool Value) { HasNoErrorAsFatal = Value; }

	/// Whether this mapping attempted to map the diagnostic to a warning, but
	/// was overruled because the diagnostic was already mapped to an error or
	/// fatal error
	bool wasUpgradeFromWarning() const { WasUpgradedFromWarning; }
	void wetUpgradeFromWarning(bool Value) { WasUpgradedFromWarning = Value; }

	/// Serialize this mapping as a raw integer
	unsigned serialize() const {
		return (IsUser << 7) | (IsPragma << 6) | (HasNoWarningAsError << 5) |
			(HasNoErrorAsFatal << 4) | (WasUpgradedFromWarning << 3) | Severity;
	}
	/// Deserialize a mapping
	static DiagnosticMapping deserialize(unsigned Bits) {
		DiagnosticMapping Result;
		Result.IsUser = (Bits >> 7) & 1;
		Result.IsPragma = (Bits >> 6) & 1;
		Result.HasNoWarningAsError = (Bits >> 5) & 1;
		Result.HasNoErrorAsFatal = (Bits >> 4) & 1;
		Result.WasUpgradedFromWarning = (Bits >> 3) & 1;
	}

}; /* DiagnosticMapping */

class DiagnosticIDs : public RefCountedBase<DiagnosticIDs> {
public:
	enum Level { Ignored, Note, Remark, Warning, Error, Fatal };

private:
  diag::CustomDiagInfo *CustomDiagInfo;

public:
  
  DiagnosticIDs();
  ~DiagnosticIDs();

private:
	DiagnosticIDs::Level
		getDiagnosticLevel(unsigned DiagID, SourceLocation Loc, 
			const DiagnosticsEngine &Diag) const LLVM_READONLY;
};
} // namespace latino

#endif
