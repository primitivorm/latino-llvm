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

} // namespace diag

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
