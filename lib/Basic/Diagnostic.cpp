#include "latino/Basic/Diagnostic.h"
#include "latino/Basic/DiagnosticError.h"
#include "latino/Basic/DiagnosticIDs.h"
#include "latino/Basic/DiagnosticOptions.h"

using namespace latino;

DiagnosticIDs::DiagnosticIDs() { CustomDiagInfo = nullptr; }
DiagnosticIDs::~DiagnosticIDs() { delete CustomDiagInfo; }
