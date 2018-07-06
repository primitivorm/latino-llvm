#include "latino/Basic/Diagnostic.h"
#include "latino/Basic/DiagnosticError.h"
#include "latino/Basic/DiagnosticIDs.h"
#include "latino/Basic/DiagnosticOptions.h"

using namespace latino;

DiagnosticsEngine::DiagnosticsEngine(
	IntrusiveRefCntPtr<DiagnosticIDs> diags,
	IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts, DiagnosticConsumer *client,
	bool ShouldOwnClient)
	:Diags(std::move(diags)), DiagOpts(std::move(DiagOpts)) {
	setClient(client, ShouldOwnClient);
	ArgToStringFn = DummyArgToStringFn;
	Reset();
}

DiagnosticsEngine::~DiagnosticsEngine() {
	setClient(nullptr);
}

DiagnosticIDs::DiagnosticIDs() { CustomDiagInfo = nullptr; }
DiagnosticIDs::~DiagnosticIDs() { delete CustomDiagInfo; }


bool DiagnosticsEngine::EmitCurrentDiagnostic(bool Force) {
	assert(getClient() && "DiagnosticClient not set!");

	bool Emitted;
	if (Force) {
		Diagnostic Info(this);
		DiagnosticIDs::Level DiagLevel
			= Diags->getDiagnosticLevel(Info.getID(), Info.getLocation(), *this);

		Emitted = (DiagLevel != DiagnosticIDs::Ignored);
		if (Emitted) {
			Diags->EmitDiag(*this, DiagLevel);
		}
	}
	else {
		Emitted = ProcessDialog();
	}
	Clear();
	if (!Force && DelayedDiagID) ReportDelayed();
	return Emitted;
}
