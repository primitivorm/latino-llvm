#ifndef LATINO_BASIC_DIAGNOSTIC_H
#define LATINO_BASIC_DIAGNOSTIC_H

#include "latino/Basic/DiagnosticIDs.h"
#include "latino/Basic/DiagnosticOptions.h"
#include "latino/Basic/SourceLocation.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

namespace latino {
class DiagnosticOptions;

class Diagnostic {
	const DiagnosticEngine *DiagObj;
	StringRef StoredDiagMessage;
public:
	explicit Diagnostic(const DiagnosticEngine *DO) : DiagObj(DO){}
	Diagnostic(const DiagnosticEngine *DO, StringRef storedDiagMessage): DiagObj(DO), StoredDiagMessage(storedDiagMessage){}
};

class DiagnosticEngine : public RefCountedBase<DiagnosticEngine> {

private:
	friend class PartialDiagnostic;
	bool SuppressAllDiagnostics;
	enum {
		MaxArguments = 10
	};
public:
	enum Level {
		Ignored = DiagnosticIDs::Ignored,
		Note = DiagnosticIDs::Note,
		Remark = DiagnosticIDs::Remark,
		Warning = DiagnosticIDs::Warning,
		Error = DiagnosticIDs::Error,
		Fatal = DiagnosticIDs::Fatal,
	};	

	unsigned TrapNumErrorsOcurred;
	unsigned TrapNumUnrecoverableErrorsOcurred;

	void setSuppressAllDiagnostics(bool Val = true) {
		SuppressAllDiagnostics = Val;
	}
};
class DiagnosticErrorTrap {
  DiagnosticEngine &Diag;
  

public:
	unsigned NumErrors;
	unsigned NumUnrecoverableErrors;
  explicit DiagnosticErrorTrap(DiagnosticEngine &Diag) : Diag(Diag) { reset(); }

  void reset() {
	  NumErrors = Diag.TrapNumErrorsOcurred;
	  NumUnrecoverableErrors = Diag.TrapNumUnrecoverableErrorsOcurred;
  }
};

class DiagnosticConsumer {
public:
  DiagnosticConsumer() = default;
  virtual ~DiagnosticConsumer();

  virtual void HandleDiagnostic(DiagnosticEngine::Level DiagLevel, const Diagnostic &Info);
};

class IgnoringDiagConsumer : public DiagnosticConsumer {
  virtual void anchor();
  void HandleDiagnostic(DiagnosticEngine::Level DiagLevel,
                        const Diagnostic &Info) override {}
};

} // namespace latino

#endif
