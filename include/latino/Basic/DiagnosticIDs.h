#ifndef LATINO_BASIC_DIAGNOSTICIDS_H
#define LATINO_BASIC_DIAGNOSTICIDS_H

#include "latino/Basic/LLVM.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/StringRef.h"
#include <vector>

namespace latino {
class DiagnosticEngine;
class SourceLocation;
namespace diag {
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
private:
  diag::CustomDiagInfo *CustomDiagInfo;

public:
  enum Level { Ignored, Note, Remark, Warning, Error, Fatal };
  DiagnosticIDs();
  ~DiagnosticIDs();
};
} // namespace latino

#endif
