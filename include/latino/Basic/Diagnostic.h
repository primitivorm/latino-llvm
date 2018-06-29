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
#include "llvm/ADT/iterator_range.h"
#include "llvm/Support/Compiler.h" 

#include <cassert>
#include <cstdint>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace latino {
class DiagnosticOptions;
class DiagnosticBuilder;

namespace tok {
	enum TokenKind : unsigned short;
}

/// Annotates a diagnostic with some code that should be
/// inserted, removed, or replaced to fix the problem.
///
/// This kind of hint should be used when we are certain that the
/// introduction, removal, or modification of a particular (small!)
/// amount of code will correct a compilation error. The compiler
/// should also provide full recovery from such errors, such that
/// suppressing the diagnostic output can still result in successful
/// compilation.
class FixItHint {
public:
	/// Code that should be replaced to correct the error. Empty for an
    /// insertion hint.
	CharSourceRange RemoveRange;

	/// Code in the specific range that should be inserted in the insertion
  	/// location.
	CharSourceRange InsertFromRange;

	/// The actual code to insert at the insertion location, as a
  	/// string.
	std::string CodeToInsert;

	bool BeforePreviousInsertions = false;

	/// Empty code modification hint, indicating that no code
  	/// modification is known.
	FixItHint() = default;

	bool isNull() const {
		return !RemoveRange.isValid();
	}

	/// Create a code modification hint that inserts the given
  	/// code string at a specific location.
	static FixItHint CreateInsertion(SourceLocation InsertionLoc, 
									 StringRef Code, 
									 bool BeforePreviousInsertions = false){
		FixItHint Hint;
		Hint.RemoveRange = 
			CharSourceRange::getCharRange(InsertionLoc, InsertionLoc);
		Hint.CodeToInsert = Code;
		Hint.BeforePreviousInsertions = BeforePreviousInsertions;
		return Hint;
	}

	/// Create a code modification hint that inserts the given
  	/// code from \p FromRange at a specific location.
	static FixItHint CreateInsertionFromRange(SourceLocation InsertionLoc,
	CharSourceRange FromRange, bool BeforePreviousInsertions = false){
		FixItHint Hint;
		Hint.RemoveRange = CharSourceRange(InsertionLoc, InsertionLoc);
		Hint.BeforePreviousInsertions = BeforePreviousInsertions;
		return Hint;
	}

	/// Create a code modification hint that removes the given
  	/// source range.
	static FixItHint CreateRemoval(CharSourceRange RemoveRange) {
		FixItHint Hint;
		Hint.RemoveRange = RemoveRange;
		return Hint;
	}

	static FixItHint CreateRemoval(SourceRange RemoveRange){
		return CreateRemoval(CharSourceRange::getTokenRange(RemoveRange));
	}

	/// Create a code modification hint that replaces the given
  	/// source range with the given code string.
	static FixItHint CreateReplacement(CharSourceRange RemoveRange, StringRef Code){
		FixItHint Hint;
		Hint.RemoveRange = RemoveRange;
		Hint.CodeToInsert = Code;
		return Hint; 
	}

	static  FixItHint CreateReplacement(SourceRange RemoveRange, StringRef Code){
		return CreateReplacement(CharSourceRange::getTokenRange(RemoveRange), Code);
	}

};

/// Concrete class used by the front-end to report problems and issues.
///
/// This massages the diagnostics (e.g. handling things like "report warnings
/// as errors" and passes them off to the DiagnosticConsumer for reporting to
/// the user. DiagnosticsEngine is tied to one translation unit and one
/// SourceManager.
class DiagnosticsEngine : public RefCountedBase<DiagnosticsEngine> {
public:
	/// The level of the diagnostic, after it has been through mapping.
	enum Level {
		Ignored = DiagnosticIDs::Ignored,
		Note = DiagnosticIDs::Note,
		Remark = DiagnosticIDs::Remark,
		Warning = DiagnosticIDs::Warning,
		Error = DiagnosticIDs::Error,
		Fatal = DiagnosticIDs::Fatal,
	};

	enum ArgumentKind {
		/// std::string
		ak_std_string,

		/// const char *
		ak_c_string,

		/// int
		ak_sint,

		/// unsigned
		ak_uint,

		/// enum TokenKind : unsigned
		ak_tokenkind,

		/// IdentifierInfo
		ak_identifierinfo,

		/// QualType
		ak_qualtype,

		/// DeclarationName
		ak_declarationname,

		/// NamedDecl *
		ak_nameddecl,

		/// NestedNameSpecifier *
		ak_nestednamespec,

		/// DeclContext *
		ak_declcontext,

		/// pair<QualType, QualType>
		ak_qualtype_pair,

		/// Attr *
		ak_attr
	};

	/// Represents on argument value, which is a union discriminated
	/// by ArgumentKind, with a value.
	using ArgumentValue = std::pair>ArgumentKind, intptr_t>;

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
