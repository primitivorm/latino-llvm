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
class DiagnosticConsumer;

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
		Hint.RemoveRange = CharSourceRange::getCharRange(InsertionLoc, InsertionLoc);
		Hint.InsertFromRange = FromRange;
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
	using ArgumentValue = std::pair<ArgumentKind, intptr_t>;

private:
	//Used by __extension__
	unsigned char AllExtensionsSilenced = 0;

	//Suppress diagnostics after a fatal error?
	bool SuppressAfterFatalError = true;

	//Suppress all diagnostics
	bool SuppressAllDiagnostics = false;

	//Ellide common types of templates
	bool ElideType = true;

	//Print a tree when comparing templates
	bool PrintTemplateTree = false;

	//Color printing is enabled
	bool ShowColors = false;

	//Which overload candidates to show
	OverloadsShown ShowOverloads = Ovl_All;

	//Cap of # errors emitted, 0 -> no limit
	unsigned TemplateBacktraceLimit = 0;

	//Cap on depth of constexpr evaluation backtrace stack, 0 -> no limit
	unsigned ConstexprBacktraceLimit = 0;

	IntrusiveRefCntPtr<DiagnosticIDs> Diags;
	IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts;
	DiagnosticConsumer *Client = nullptr;
	std::unique_ptr<DiagnosticConsumer> Owner;
	SourceManager *SourceMgr = nullptr;

	/// Mapping information for diagnostics.
	///
	/// Mapping info is packed into four bits per diagnostic.  The low three
	/// bits are the mapping (an instance of diag::Severity), or zero if unset.
	/// The high bit is set when the mapping was established as a user mapping.
	/// If the high bit is clear, then the low bits are set to the default
	/// value, and should be mapped with -pedantic, -Werror, etc.
	///
	/// A new DiagState is created and kept around when diagnostic pragmas modify
	/// the state so that we know what is the diagnostic state at any given
	/// source location.
	class DiagState {
		llvm::DenseMap<unsigned, DiagnosticMapping> DiagMap;

	public:
		// "Global" configuration state that can actually vary between modules

		// Ignore all warnings: -w
		unsigned IgnoreAllWarnings : 1;

		// Enable all warnings
		unsigned EnableAllWarnings : 1;

		//Treat warnings like errors
		unsigned WarningsAsErrors : 1;

		// Treat errors like fatal errors
		unsigned ErrorsAsFatal : 1;

		// Suppress warnings in system headers
		unsigned SuppressSystemWarnings : 1;

		// Map extensions to warnings or errors?
		diag::Severity ExtBehavior = diag::Severity::Ignored;

		DiagState()
			:IgnoreAllWarnings(false), EnableAllWarnings(false),
			WarningsAsErrors(false), ErrorsAsFatal(false),
			SuppressSystemWarnings(false) {}

		using iterator = llvm::DenseMap<unsigned, DiagnosticMapping>::iterator;
		using const_iterator = llvm::DenseMap <unsigned, DiagnosticMapping>:: const_iterator;

		void setMapping(diag::kind Diag, DiagnosticMapping Info) {
			DiagMap[Diag] = Info;
		}

		DiagnosticMapping lookupMapping(diag::kind Diag) const {
			return DiagMap.lookup(Diag);
		}

		DiagnosticMapping &getOrAddMapping(diag::kind Diag);

		const_iterator begin() const { return DiagMap.begin(); }
		const_iterator end() const { return DiagMap.end(); }

	}; /* DiagState */

	/// Keeps and automatically disposes all DiagStates that we create
	std::list<DiagState> DiagStates;

	/// A mapping from files to the diagnostic states for those files. Lazily
	/// built on demand for files in which the diagnostic state has not changed
	class DiagStateMap {
	public:
		/// Add an initial latest state point
		void appendFirst(DiagState *State);

		/// Add a new latest state point
		void append(SourceManager &SrcMgr, SourceLocation Loc, DiagState *State);

		/// Look up the diagnostic state at a given source location
		DiagState *lookup(SourceManager &SrcMgr, SourceLocation Loc) const;

		/// Determine whether this map is empty
		bool empty() const { return Files.empty(); }

		/// Clear out this map
		void clear() {
			Files.clear();
			FirstDiagState = CurDiagState = nullptr;
			CurDiagStateLoc = SourceLocation();
		}

		/// Produce a debugging dump of the diagnostic state
		LLVM_DUMP_METHOD void dump(SourceManager &SrcMgr, StringRef DiagName = StringRef()) const;

		/// Grab the most-recently-added state point
		DiagState *getCurDiagState() const { return CurDiagState; }

		/// Get the location at which a diagnostic state was last added
		SourceLocation getCurDiagStateLoc() const { return CurDiagStateLoc; }

	private:
		//friend class ASTReader;
		//friend class ASTWriter;

		/// Represents a point in source where the diagnostic state was
		/// modified because of a pragma.
		///
		/// 'Loc' can be null if the point represents the diagnostic state
		/// modifications done through the command-line.
		struct DiagStatePoint {
			DiagState *State;
			unsigned Offset;

			DiagStatePoint(DiagState *State, unsigned Offset)
				:State(State), Offset(Offset);
		};

		/// Description of the diagnostic states and state transitions for a
		/// particular FileID.
		struct File {
			/// The diagnostic state for the parent file. This is strictly redundant,
			/// as looking up the DecomposedIncludedLoc for the FileID in the Files
			/// map would give us this, but we cache it here for performance
			File *Parent = nullptr;

			/// The offset of this file within its parent
			unsigned ParentOffset = 0;

			/// Whether this file has any local (not imported from an AST file)
			/// diagnostic state transitions
			bool HasLocalTransitions = false;

			/// The points within the file where the state changes. There will always
			/// be at least one of these (the state on entry to the file)
			llvm::SmallVector<DiagStatePoint, 4> StateTransitions;

			DiagState *lookup(unsigned Offset) const;

		};

		/// The diagnostic states for each file
		mutable std::map<FileID, File> Files;

		/// The initial diagnostic state
		DiagState *FirstDiagState;

		/// The current diagnostic state
		DiagState *CurDiagState;

		/// The location at which the current diagnostic state was established
		SourceLocation CurDiagStateLoc;

		/// Get the diagnostic state information for a file
		File *getFile(SourceManager &SrcMgr, FileID ID) const;
	};

	DiagStateMap DiagStatesByLoc;

	/// Keeps the DiagState that was active during each diagnostic 'push'
	/// so we can get back at it when we 'pop'
	std::vector<DiagState *> DiagStateOnPushStack;
	
	DiagState *GetCurDiagState() const {
		return DiagStatesByLoc.getCurDiagState();
	}

	void PushDiagStatePoint(DiagState *State, SourceLocation L);

	/// Finds the DiagStatePoint that contains the diagnostic state of
	/// the given source location
	DiagState *GetDiagStateLoc(SourceLocation Loc) const {
		return SourceMgr ? DiagStatesByLoc.lookup(*SourceMgr, Loc)
			: DiagStatesByLoc.getCurDiagState();
	}

	/// Sticky flag set to \c true when an error is emitted
	bool ErrorOcurred;

	/// Sticky flag set to \c true when an "uncompilable error" occurs
	/// I.e. an error that was not upgraded from a warning by -Werror
	bool UncompilableErroOcurred;

	/// Sticky flag set to \c true when a fatal error is emitted
	bool FatalErrorOcurred;

	/// Indicates that an unrecoverable error has occurred
	bool UnrecoverableErrorOcurred;

	/// Counts for DiagnosticErrorTrap to check whether an error occurred
	/// during a parsing section, e.g. during parsing a function
	unsigned TrapNumErrorsOcurred;
	unsigned TrapNumUnrecoverableErrorOcurred;

	/// The level of the last diagnostic emitted.
	///
	/// This is used to emit continuation diagnostics with the same level as the
	/// diagnostic that they follow
	DiagnosticIDs::Level LastDiagLevel;

	/// Number of warnings reported
	unsigned NumWarnings;

	/// Number of errors reported
	unsigned NumErrors;

	/// A function pointer that converts an opaque diagnostic
	/// argument to a strings.
	///
	/// This takes the modifiers and argument that was present in the diagnostic.
	///
	/// The PrevArgs array indicates the previous arguments formatted for this
	/// diagnostic.  Implementations of this function can use this information to
	/// avoid redundancy across arguments.
	///
	/// This is a hack to avoid a layering violation between libbasic and libsema
	using ArgToStringFnTy = void(*)(
		ArgumentKind Kind, intptr_t Val,
		StringRef Modifier, StringRef Argument,
		ArrayRef<ArgumentValue> PrevArgs,
		SmallVectorImpl<char> &Output,
		void *Cookie,
		ArrayRef<intptr_t> QualTypeVals);

	void *ArgToStringCookie = nullptr;
	ArgToStringFnTy ArgToStringFn;

	/// ID of the "delayed" diagnostic, which is a (typically
	/// fatal) diagnostic that had to be delayed because it was found
	/// while emitting another diagnostic
	unsigned DelayedDiagID;

	/// First string argument for the delayed diagnostic
	std::string DelayedDiagArg1;

	/// Second string argument for the delayed diagnostic
	std::string DelayedDiagArg2;

	/// Optional flag value.
	///
	/// Some flags accept values, for instance: -Wframe-larger-than=<value> and
	/// -Rpass=<value>. The content of this string is emitted after the flag name
	/// and '='
	std::string FlagValue;

public:
	explicit DiagnosticsEngine(IntrusiveRefCntPtr<DiagnosticIDs> Diags,
		IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts,
		DiagnosticConsumer *client = nullptr,
		bool ShouldOwnClient = true);
	DiagnosticsEngine(const DiagnosticsEngine &) = delete;
	DiagnosticsEngine &operator=(const DiagnosticsEngine &) = delete;
	~DiagnosticsEngine();

	LLVM_DUMP_METHOD void dump() const { DiagStatesByLoc.dump(*SourceMgr); }
	LLVM_DUMP_METHOD void dump(StringRef DiagName) const {
		DiagStatesByLoc.dump(*SourceMgr, DiagName);
	}

	const IntrusiveRefCntPtr<DiagnosticIDs> &getDiagnosticIDs() const {
		return Diags;
	}

	/// Retrieve the diagnostic options
	DiagnosticOptions &getDiagnosticsOptions() const { return *DiagOpts; }

	using diag_mapping_range = llvm::iterator_range<DiagState::const_iterator>;

	/// Get the current set of diagnostic mappings
	diag_mapping_range getDiagnosticMapping() const {
		const DiagState &DS = *GetCurDiagState();
		return diag_mapping_range(DS.begin(), DS.end());
	}

	DiagnosticConsumer *getClient() { return Client; }
	const DiagnosticConsumer *getClient() const { return Client; }

	/// Determine whether this \c DiagnosticsEngine object own its client
	bool ownsClient() const { return Owner != nullptr; }

	/// Return the current diagnostic client along with ownership of that
	/// client
	std::unique_ptr<DiagnosticConsumer> takeClient() { return std::move(Owner); }

	bool hasSourceManager() const { return SourceMgr != nullptr; }

	SourceManager &getSourceManager() const {
		assert(SourceMgr && "SourceManager not set!");
		return *SourceMgr;
	}

	void setSourceManager(SourceManager *SrcMgr) {
		assert(DiagStatesByLoc.empty() && "Leftover diag state from different SourceManager");
		SourceMgr = SrcMgr;
	}

	//===--------------------------------------------------------------------===//
	//  DiagnosticsEngine characterization methods, used by a client to customize
	//  how diagnostics are emitted.
	//

	/// Copies the current DiagMappings and pushes the new copy
	/// onto the top of the stack

	void pushMappings(SourceLocation Loc);

	/// Pops the current DiagMappings off the top of the stack,
	/// causing the new top of the stack to be the active mappings.
	///
	/// \returns \c true if the pop happens, \c false if there is only one
	/// DiagMapping on the stack
	bool popMappings(SourceLocation Loc);

	/// Set the diagnostic client associated with this diagnostic object.
	///
	/// \param ShouldOwnClient true if the diagnostic object should take
	/// ownership of \c client
	void setClient(DiagnosticConsumer *client, bool ShouldOwnClient = true);

	/// Specify a limit for the number of errors we should
	/// emit before giving up.
	///
	/// Zero disables the limit
	void setTemplateBacktraceLimit(unsigned Limit) {
		TemplateBacktraceLimit = Limit;
	}

	/// Retrieve the maximum number of template instantiation
	/// notes to emit along with a given diagnostic
	unsigned getTemplateBacktraceLimit() const {
		return TemplateBacktraceLimit;
	}

	/// Specify the maximum number of constexpr evaluation
	/// notes to emit along with a given diagnostic
	void setContexpBacktraceLimit(unsigned Limit) {
		ConstexprBacktraceLimit = Limit;
	}

	unsigned getContexpBacktraceLimit() const {
		return ConstexprBacktraceLimit;
	}

	/// When set to true, any unmapped warnings are ignored.
	///
	/// If this and WarningsAsErrors are both set, then this one wins
	void setIgnoreAllWarnings(bool Val) {
		GetCurDiagState()->IgnoreAllWarnings = Val;
	}
	bool getIgnoreAllWarnings() const {
		return GetCurDiagState()->IgnoreAllWarnings;
	}

	/// When set to true, any unmapped ignored warnings are no longer
	/// ignored.
	///
	/// If this and IgnoreAllWarnings are both set, then that one wins
	void setEnableAllWarnings(bool Val) {
		GetCurDiagState()->EnableAllWarnings = Val;
	}
	bool getEnableAllWarnings() const {
		return GetCurDiagState()->EnableAllWarnings;
	}

	/// When set to true, any warnings reported are issued as errors
	void setWarningsAsErrors(bool Val) {
		GetCurDiagState()->WarningsAsErrors = Val;
	}
	bool getWarningsAsErrors() const {
		return GetCurDiagState()->WarningsAsErrors;
	}

	/// When set to true, any error reported is made a fatal error
	void setErrorAsFatal(bool Val) { GetCurDiagState()->ErrorsAsFatal = Val; }
	bool getErrorAsFatal() const { return GetCurDiagState()->ErrorsAsFatal; }

	/// When set to true (the default), suppress further diagnostics after
	/// a fatal error
	void setSuppressAfterFatalError(bool Val) { SuppressAfterFatalError = Val; }

	/// When set to true mask warnings that come from system headers
	void setSuppressSystemWarnings(bool Val) {
		GetCurDiagState()->SuppressSystemWarnings = Val;		
	}
	bool getSuppressSystemWarnings() const {
		return GetCurDiagState()->SuppressSystemWarnings;
	}

	/// Suppress all diagnostics, to silence the front end when we 
	/// know that we don't want any more diagnostics to be passed along to the
	/// client
	void setSuppressAllDiagnostics(bool Val = true) {
		SuppressAllDiagnostics = Val;
	}
	bool getSuppressAllDiagnostics() const { return SuppressAllDiagnostics; }

	/// Set type eliding, to skip outputting same types occurring in
	/// template types
	void setElideType(bool Val = true) { ElideType = Val; }
	bool getElideType() const { return ElideType; }

	/// Set tree printing, to outputting the template difference in a
	/// tree format
	void setPrintTemplateTree(bool Val = true) { PrintTemplateTree = Val; }
	//TODO:


	friend class PartialDiagnostic;
	friend class DiagnosticBuilder;
	
	bool SuppressAllDiagnostics = false;
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
protected:
	unsigned NumWarnings = 0;
	unsigned NumErrors = 0;

public:
  DiagnosticConsumer() = default;
  virtual ~DiagnosticConsumer();

  unsigned getNumErrors() const { return NumErrors; }
  unsigned getNumWarnings() const { return NumWarnings; }
  virtual void clear() { NumWarnings = NumErrors = 0; }

  /// Callback to inform the diagnostic client that processing
  /// of a source file is beginning.
  ///
  /// Note that diagnostics may be emitted outside the processing of a source
  /// file, for example during the parsing of command line options. However,
  /// diagnostics with source range information are required to only be emitted
  /// in between BeginSourceFile() and EndSourceFile().
  ///
  /// \param LangOpts The language options for the source file being processed.
  /// \param PP The preprocessor object being used for the source; this is 
  /// optional, e.g., it may not be present when processing AST source files.
  //virtual void BeginSourceFile(const LangOptions &LangOpts, const Preprocessor *PP = nullptr) {}

  /// Callback to inform the diagnostic client that processing
  /// of a source file has ended
  ///
  /// The diagnostic client should assume that any objects made available via
  /// BeginSourceFile() are inaccessible
  virtual void EndSourceFile() {}

  /// Callback to inform the diagnostic client that processing of all
  /// source files has ended
  virtual void finish() {}

  /// Indicates whether the diagnostics handled by this
  /// DiagnosticConsumer should be included in the number of diagnostics
  /// reported by DiagnosticsEngine
  ///
  /// The default implementation returns true
  virtual bool IncludeInDiagnosticCounts() const;

  /// Handle this diagnostic, reporting it to the user or
  /// capturing it to a log as needed
  ///
  /// The default implementation just keeps track of the total number of
  /// warnings and errors
  virtual void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel, const Diagnostic &Info);
};

/// A diagnostic client that ignores all diagnostics
class IgnoringDiagConsumer : public DiagnosticConsumer {
  virtual void anchor();
  void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel,
                        const Diagnostic &Info) override {
	  // Just ignore it
  }
}; /* IgnoringDiagConsumer */

/// Diagnostic consumer that forwards diagnostics along to an
/// existing, already-initialized diagnostic consumer.
///
class ForwardingDiagnosticConsumer : public DiagnosticConsumer {
	DiagnosticConsumer &Target;

public:
	ForwardingDiagnosticConsumer(DiagnosticConsumer &Target) : Target(Target) {}
	~ForwardingDiagnosticConsumer() override;

	void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel, const Diagnostic &Info) override;

	void clear() override;

	bool IncludeInDiagnosticCounts() const override;

};

// Struct used for sending info about how a type should be printed
struct TemplateDiffTypes {
	intptr_t FromType;
	intptr_t ToType;
	unsigned PrintTree : 1;
	unsigned PrintFromType : 1;
	unsigned ElideType : 1;
	unsigned ShowColors : 1;

	// The printer sets this variable to true if the template diff was used
	unsigned TemplateDiffUsed : 1;
};

/// Special character that the diagnostic printer will use to toggle the bold
/// attribute.  The character itself will be not be printed.
const char ToggleHighlight = 127;

/// ProcessWarningOptions - Initialize the diagnostic client and process the
/// warning options specified on the command line
void ProcessWarningsOptions(DiagnosticsEngine &Diags,
	const DiagnosticOptions &Opts,
	bool ReportDiags = true);

} // namespace latino

#endif /* LATINO_BASIC_DIAGNOSTIC_H */
