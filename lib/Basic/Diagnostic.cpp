#include "latino/Basic/Diagnostic.h"
#include "latino/Basic/CharInfo.h"
#include "latino/Basic/DiagnosticError.h"
#include "latino/Basic/DiagnosticIDs.h"
#include "latino/Basic/DiagnosticOptions.h"
#include "latino/Basic/IdentifierTable.h"
#include "latino/Basic/PartialDiagnostic.h"
#include "latino/Basic/SourceLocation.h"
#include "latino/Basic/SourceManager.h"
#include "latino/Basic/Specifiers.h"
#include "latino/Basic/TokenKinds.h"
#include "llvm/ADT/SmallRef.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/CrashRecoveryContext.h"
#include "llvm/Support/Locale.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <limits>
#include <string>
#include <utility>
#include <vector>

using namespace latino;

const DiagnosticBuilder &latino::operator<<(const DiagnosticBuilder &DB,
                                            DiagNullabilityKind nullability) {
  StringRef string;
  switch (nullability.first) {
  case NullabilityKind::NonNull:
    string = nullability.second ? "'nonnull'" : "'_Nonnull'";
    break;
  case NullabilityKind::Nullable:
    string = nullability.second ? "'nullable'" : "'_Nullable'";
    break;
  case NullabilityKind::Unspecified:
    string = nullability.second ? "'null_unspecified'" : "'_Null_unspecified'";
    break;
  }
  DB.AddString(string);
  return DB;
}

static void
DummyArgToStringFn(DiagnosticsEngine::ArgumentKind AK, intptr_t QT,
                   StringRef Modifier, StringRef Argument,
                   ArrayRef<DiagnosticsEngine::ArgumentValue> PrevArgs,
                   SmallVectorImpl<char> &Output, void *Cookie,
                   ArrayRef<intptr_t> QualTypeVals) {
  StringRef Str = "<can´t format argument>";
  Output.append(Str.begin(), Str.end());
}

DiagnosticsEngine::DiagnosticsEngine(
    IntrusiveRefCntPtr<DiagnosticIDs> diags,
    IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts, DiagnosticConsumer *client,
    bool ShouldOwnClient)
    : Diags(std::move(diags)), DiagOpts(std::move(DiagOpts)) {
  setClient(client, ShouldOwnClient);
  ArgToStringFn = DummyArgToStringFn;

  Reset();
}

DiagnosticsEngine::~DiagnosticsEngine() {
  // If we own the diagnostic client, destroy it first so that it can access the
  // engine from its destructor.
  setClient(nullptr);
}

void DiagnosticsEngine::setClient(DiagnosticConsumer *client,
                                  bool ShouldOwnClient) {
  Owner.reset(ShouldOwnClient ? client : nullptr);
  Client = client;
}

void DiagnosticsEngine::pushMappings(SourceLocation Loc) {
  DiagStateOnPushStack.push_back(GetCurDiagState());
}

bool DiagnosticsEngine::popMappings(SourceLocation Loc) {
  if (DiagStateOnPushStack.empty())
    return false;
  if (DiagStateOnPushStack.back() != GetCurDiagState()) {
    // State changed at some point between push/pop.
    PushDiagStatePoint(DiagStateOnPushStack.back(), Loc);
  }
  DiagStateOnPushStack.pop_back();
  return true;
}

void DiagnosticsEngine::Reset() {
  ErrorOcurred = false;
  UncompilableErroOcurred = false;
  FatalErrorOcurred = false;
  UnrecoverableErrorOcurred = false;

  NumWarnings = 0;
  NumErrors = 0;
  TrapNumErrorsOcurred = 0;
  TrapNumUnrecoverableErrorOcurred = 0;

  CurDiagID = std::numeric_limits<unsigned>::max();
  LastDiagLevel = DiagnosticIDs::Ignored;
  DelayedDiagID = 0;

  // Clear state related to #pragma diagnostic.
  DiagStates.clear();
  DiagStatesByLoc.clear();
  DiagStateOnPushStack.clear();

  // Create a DiagState and DiagStatePoint representing diagnostic changes
  // through command-line.
  DiagStates.emplace_back();
  DiagStatesByLoc.appendFirst(&DiagStates.back());
}

void DiagnosticsEngine::SetDelayedDiagnostic(unsigned DiagID, StringRef Arg1,
                                             StringRef Arg2) {
  if (DelayedDiagID)
    return;

  DelayedDiagID = DiagID;
  DelayedDiagArg1 = Arg1;
  DelayedDiagArg2 = Arg2;
}

void DiagnosticsEngine::ReportDelayed() {
  unsigned ID = DelayedDiagID;
  DelayedDiagID = 0;
  Report(ID) << DelayedDiagArg1 << DelayedDiagArg2;
}

void DiagnosticsEngine::DiagStateMap::appendFirst(DiagState *State) {
  assert(Files.empty() && "not first");
  FirstDiagState = CurDiagState = State;
  CurDiagStateLoc = SourceLocation();
}

void DiagnosticsEngine::DiagStateMap::append(SourceManager &SrcMgr,
                                             SourceLocation Loc,
                                             DiagState *State) {
  CurDiagState = State;
  CurDiagStateLoc = Loc;

  std::pair<FileID, unsigned> Decomp = SrcMgr.getDecomposedLoc(Loc);
  unsigned Offset = Decomp.second;
  for (File *F = getFile(SrcMgr, Decomp.first); F;
       Offset = F->ParentOffset, F = F->Parent) {
    F->HasLocalTransitions = true;
    auto &Last = F->StateTransitions.back();
    assert(Last.Offset <= Offset && "state transitions added out of order");

    if (Last.Offset == Offset) {
      if (Last.State == State)
        break;
      Last.State = State;
      continue;
    }

    F->StateTransitions.push_back({State, Offset});
  }
}

DiagnosticsEngine::DiagState *
DiagnosticsEngine::DiagStateMap::File::lookup(unsigned Offset) const {
  auto OnePastIt =
      std::upper_bound(StateTransitions.begin(), StateTransitions.end(), Offset,
                       [](unsigned Offset, const DiagStatePoint &P) {
                         return Offset < P.Offset;
                       });
  assert(OnePastIt != StateTransitions.begin() && "missing initial state");
  return OnePastIt[-1].State;
}

DiagnosticsEngine::DiagStateMap::File *
DiagnosticsEngine::DiagStateMap::getFile(SourceManager &SrcMgr,
                                         FileID ID) const {
  // Get or insert the File for this ID.
  auto Range = Files.equal_range(ID);
  if (Range.first != Range.second)
    return &Range.first->second;
  auto &F = Files.insert(Range.first, std::make_pair(ID, File()))->second;

  // We created a new File; look up the diagnostic state at the start of it and
  // initialize it.
  if (ID.isValid()) {
    std::pair<FileID, unsigned> Decomp = SrcMgr.getDecomposedIncludedLoc(ID);
    F.Parent = getFile(SrcMgr, Decomp.first);
    F.ParentOffset = Decomp.second;
    F.StateTransitions.push_back({F.Parent->lookup(Decomp.second), 0});
  } else {
    // This is the (imaginary) root file into which we pretend all top-level
    // files are included; it descends from the initial state.
    //
    // FIXME: This doesn't guarantee that we use the same ordering as
    // isBeforeInTranslationUnit in the cases where someone invented another
    // top-level file and added diagnostic pragmas to it. See the code at the
    // end of isBeforeInTranslationUnit for the quirks it deals with.
    F.StateTransitions.push_back({FirstDiagState, 0});
  }
  return &F;
}

void DiagnosticsEngine::DiagStateMap::dump(SourceManager &SrcMgr,
                                           StringRef DiagName) const {
  llvm::errs() << "diagnostic state at ";
  CurDiagStateLoc.dump(SrcMgr);
  llvm::errs() << ":" << CurDiagState << "\n";

  for (auto &F : Files) {
    FileID ID = F.first;
    File &File = F.second;

    bool PrintedOuterHeading = false;
    auto PrintOuterHeading = [&] {
      if (PrintedOuterHeading)
        return;
      PrintedOuterHeading = true;

      llvm::errs() << "File" << &File << " <FileID " << ID.getHashValue()
                   << ">: " << SrcMgr.getBuffer(ID)->getBufferIdentifier();
      if (F.second.Parent) {
        std::pair<FileID, unsigned> Decomp =
            SrcMgr.getDecomposedIncludedLoc(ID);
        assert(File.ParentOffset == Decomp.second);
        llvm::errs() << " parent " << File.Parent << " <FileID "
                     << Decomp.first.getHashValue() << "> ";
        SrcMgr.getLocForStartOfFile(Decomp.first)
            .getLocWithOffset(Decomp.first)
            .dump(SrcMgr);
      }
      if (File.HashLocalTransitions)
        llvm::errs() << " has_local_transitions";
      llvm::errs() << "\n";
    };

    if (DiagName.empty())
      PrintOuterHeading();
    for (DiagStatePoint &Transition : File.StateTransitions) {
      bool PrintedInnerHeading = false;
      auto PrintInnerHeading = [&] {
        if (PrintedInnerHeading)
          return;
        PrintedInnerHeading = true;

        PrintOuterHeading();
        llvm::errs() << " ";
        SrcMgr.getLocForStartOfFile(ID)
            .getLocWithOffset(Transition.Offset)
            .dump(SrcMgr);
        llvm::errs() << ": state " << Transition.State << ":\n";
      };

      if (DiagName.empty())
        PrintInnerHeading();

      for (auto &Mapping : *Transition.State) {
        StringRef Option =
            DiagnosticIDs::getWarningOptionForDiag(Mapping.first);
        if (!DiagName.empty() && DiagName != Option)
          continue;

        PrintInnerHeading();
        llvm::errs() << "    ";
        if (Option.empty())
          llvm::errs() << "<unknown " << Mapping.first << ">";
        else
          llvm::errs() << Option;
        llvm::errs() ": ";

        switch (Mapping.second.getSeverity()) {
        case diag::Severity::Ignored:
          llvm::errs() << "ignored";
          break;
        case diag::Severity::Remark:
          llvm::errs() << "remark";
          break;
        case diag::Severity::Warning:
          llvm::errs() << "warning";
          break;
        case diag::Severity::Error:
          llvm::errs() << "error";
          break;
        case diag::Severity::Fatal:
          llvm::errs() << "fatal";
          break;
        }

        if (!Mapping.second.isUser())
          llvm::errs() << " default";
        if (Mapping.second.isPragma)
          llvm::errs() << " pragma";
        if (Mapping.second.hasNoWarningAsError())
          llvm::errs() << " no-error";
        if (Mapping.second.hasNoErrorAsFatal())
          llvm::errs() << " no-fatal";
        if (Mapping.second.wasUpgradeFromWarning())
        llvm:
          errs() << " overruled";
        llvm::errs() << "\n";
      }
    }
  }
}

void DiagnosticsEngine::PushDiagStatePoint(DiagState *State,
                                           SourceLocation Loc) {
  assert(Loc.IsValid() && "Adding invalid loc point");
  DiagStatesByLoc.append(*SourceMgr, Loc, State);
}

void DiagnosticsEngine::PushDiagStatePoint(DiagState *State,
                                           SourceLocation Loc) {
  assert(Loc.isValid() && "Adding invalid loc point");
  DiagStatesByLoc.append(*SourceMgr, Loc, State);
}

void DiagnosticsEngine::setSeverity(diag::kind Diag, diag::Severity Map,
                                    SourceLocation L) {
  assert(Diag < diag::DIAG_UPPER_LIMIT && "Can only map builtin diagnostics");
  assert((Diags->isBuiltinWarningOrExtension(Diag) ||
          (Map == diag::Severity::Fatal || Map == diag::Severity::Error)) &&
         "Cannot map errors into warnings!");
  assert((L.isValid() || SourceMgr) && "No SourceMgr for valid location");

  // Don't allow a mapping to a warning override an error/fatal mapping.
  bool WasUpgradedFromWarning = false;
  if (Map == diag::Severity::Warning) {
    DiagnosticMapping &Info = GetCurDiagState()->getOrAddMapping(Diag);
    if (Info.getSeverity() == diag::Severity::Error ||
        Info.getSeverity() == diag::Severity::Fatal) {
      Map = Info.getSeverity();
      WasUpgradedFromWarning = true;
    }
  }
  DiagnosticMapping Mapping = makeUserMapping(Map, L);
  Mapping.setUpgradeFromWarning(WasUpgradedFromWarning);

  // Common case; setting all the diagnostics of a group in one place.
  if ((L.isValid() || L == DiagStatesByLoc.GetCurDiagStateLoc()) &&
      DiagStatesByLoc.getCurDiagState()) {
    // FIXME: This is theoretically wrong: if the current state is shared with
    // some other location (via push/pop) we will change the state for that
    // other location as well. This cannot currently happen, as we can't update
    // the diagnostic state at the same location at which we pop.
    DiagStatesByLoc.getCurDiagState()->setMapping(Diag, Mapping);
    return;
  }
  // A diagnostic pragma occurred, create a new DiagState initialized with
  // the current one and a new DiagStatePoint to record at which location
  // the new state became active.
  DiagStates.push_back(*GetCurDiagState());
  DiagStates.back().setMapping(Diag, Mapping);
  PushDiagStatePoint(&DiagStates.back(), L);
}

bool DiagnosticsEngine::EmitCurrentDiagnostic(bool Force) {
  assert(getClient() && "DiagnosticClient not set!");

  bool Emitted;
  if (Force) {
    Diagnostic Info(this);
    DiagnosticIDs::Level DiagLevel =
        Diags->getDiagnosticLevel(Info.getID(), Info.getLocation(), *this);

    Emitted = (DiagLevel != DiagnosticIDs::Ignored);
    if (Emitted) {
      Diags->EmitDiag(*this, DiagLevel);
    }
  } else {
    Emitted = ProcessDialog();
  }
  Clear();
  if (!Force && DelayedDiagID)
    ReportDelayed();
  return Emitted;
}
