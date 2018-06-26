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

class FixItHint {

public:
	FixItHint() = default;

};

class Diagnostic {
	const DiagnosticsEngine *DiagObj;
	StringRef StoredDiagMessage;
public:
	explicit Diagnostic(const DiagnosticsEngine *DO) : DiagObj(DO){}
	Diagnostic(const DiagnosticsEngine *DO, StringRef storedDiagMessage): DiagObj(DO), StoredDiagMessage(storedDiagMessage){}
};

class DiagnosticBuilder {
	friend class DiagnosticsEngine;
	friend class PartialDiagnostic;

	mutable DiagnosticsEngine *DiagObj = nullptr;
	mutable unsigned NumArgs = 0;
	mutable bool IsActive = false;
	mutable bool IsForceEmit = false;

	DiagnosticBuilder() = default;

	explicit DiagnosticBuilder(DiagnosticsEngine *diagObj)
		:DiagObj(diagObj), IsActive(true) {
		assert(diagObj && "DiagnosticBuilder requires a valid DiagnosticsEngine!");
		diagObj->DiagRanges.clear();
		diagObj->DiagFixHints.clear();
	}

protected:
	void FlushCounts() {
		DiagObj->NumDiagArgs = NumArgs;
	}
	void Clear() {
		DiagObj = nullptr;
		IsActive = false;
		IsForceEmit = false;
	}

	bool isActive() const { return IsActive; }

	bool Emit() {
		if (!isActive()) return false;
		FlushCounts();
		bool Result = DiagObj->EmitCurrentDiagnostic(IsForceEmit);
		Clear();
		return Result;
	}	

public:
	DiagnosticBuilder(const DiagnosticBuilder &D) {
		DiagObj = D.DiagObj;
		IsActive = D.IsActive;
		IsForceEmit = D.IsForceEmit;
		D.Clear();
		NumArgs = D.NumArgs;
	}

	DiagnosticBuilder &operator=(const DiagnosticBuilder &) = delete;
	~DiagnosticBuilder() {
		Emit();
	}

	void addFlagValue(StringRef V) const { DiagObj->FlagValue = V; }
};

struct AddFlagValue {
	StringRef Val;
	explicit AddFlagValue(StringRef V) : Val(V) {}
};

inline const DiagnosticBuilder &operator<<(const DiagnosticBuilder &DB,
	const AddFlagValue V) {
	DB.addFlagValue(V.Val);
	return DB;
}

class DiagnosticsEngine : public RefCountedBase<DiagnosticsEngine> {
public:
	enum Level {
		Ignored = DiagnosticIDs::Ignored,
		Note = DiagnosticIDs::Note,
		Remark = DiagnosticIDs::Remark,
		Warning = DiagnosticIDs::Warning,
		Error = DiagnosticIDs::Error,
		Fatal = DiagnosticIDs::Fatal,
	};

private:
	friend class PartialDiagnostic;
	friend class DiagnosticBuilder;

	IntrusiveRefCntPtr<DiagnosticIDs> Diags;
	IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts;

	bool SuppressAllDiagnostics;
	enum {
		MaxArguments = 10
	};

	signed char NumDiaArgs;

	SmallVector<CharSourceRange, 8> DiagRanges;
	SmallVector<FixItHint, 8> DiagFixHints;
	SourceLocation CurDiagLoc;
	unsigned CurDiagID;
	std::string FlagValue;
	signed char NumDiagArgs;

protected:
	bool EmitCurrentDiagnostic(bool Force = false);

public:
	explicit DiagnosticsEngine(IntrusiveRefCntPtr<DiagnosticIDs> Diags,
		IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts,
		DiagnosticConsumer *client = nullptr,
		bool ShouldOwnClient = true);
	DiagnosticsEngine(const DiagnosticsEngine &) = delete;
	DiagnosticsEngine &operator=(const DiagnosticsEngine &) = delete;
	~DiagnosticsEngine();

	unsigned TrapNumErrorsOcurred;
	unsigned TrapNumUnrecoverableErrorsOcurred;

	void setSuppressAllDiagnostics(bool Val = true) {
		SuppressAllDiagnostics = Val;
	}

	Level getDiagnosticLevel(unsigned DiagID, SourceLocation Loc) const {
		return (Level)Diags->getDiagnosticLevel(DiagID, Loc, *this);
	}

	inline DiagnosticBuilder DiagnosticsEngine::Report(SourceLocation Loc, unsigned DiagID) {
		assert(CurDiagID == std::numeric_limits<unsigned>::max() && 
			"Multiple diagnostics in flight at once!");
		CurDiagLoc = Loc;
		CurDiagID = DiagID;
		FlagValue.clear();
		return DiagnosticBuilder(this);
	}

	inline DiagnosticBuilder DiagnosticsEngine::Report(unsigned DiagID) {
		return Report(SourceLocation(), DiagID);
	}
};

class DiagnosticErrorTrap {
  DiagnosticsEngine &Diag;
  

public:
	unsigned NumErrors;
	unsigned NumUnrecoverableErrors;
  explicit DiagnosticErrorTrap(DiagnosticsEngine &Diag) : Diag(Diag) { reset(); }

  void reset() {
	  NumErrors = Diag.TrapNumErrorsOcurred;
	  NumUnrecoverableErrors = Diag.TrapNumUnrecoverableErrorsOcurred;
  }
};

class DiagnosticConsumer {
public:
  DiagnosticConsumer() = default;
  virtual ~DiagnosticConsumer();

  virtual void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel, const Diagnostic &Info);
};

class IgnoringDiagConsumer : public DiagnosticConsumer {
  virtual void anchor();
  void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel,
                        const Diagnostic &Info) override {}
};

} // namespace latino

#endif
