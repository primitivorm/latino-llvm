#include "latino/Basic/Diagnostic.h"
#include "latino/Basic/DiagnosticError.h"
#include "latino/Basic/DiagnosticIDs.h"
#include "gtest/gtest.h"

using namespace llvm;
using namespace latino;

namespace {
TEST(DiagnosticTest, suppressAndTrap) {
  DiagnosticsEngine Diags(new DiagnosticIDs(), new DiagnosticOptions,
                         new IgnoringDiagConsumer());
  Diags.setSuppressAllDiagnostics(true);
  {
    DiagnosticErrorTrap trap(Diags);
    Diags.Report(diag::err_target_unknown_triple) << "unknown";
    Diags.Report(diag::err_cannot_open_file) << "file"
                                             << "error";
    Diags.Report(diag::warn_mt_message) << "warning";
    EXPECT_TRUE(trap.hasErrorOccurred());
    EXPECT_TRUE(trap.hasUnrecoverableErrorOcurred());
  }
  EXPECT_FALSE(Diags.hasErrorOccurred());
  EXPECT_FALSE(Diags.hasFatalErrorOcurred());
  EXPECT_FALSE(Diags.hasUncompilableErrorOcurred());
  EXPECT_FALSE(Diags.hasUnrecoverableErrorOcurred());
}

TEST(DiagnosticTest, suppressAfterFatalError) {
	for (unsigned Suppress = 0; Suppress != 2; ++Suppress) {
    DiagnosticsEngine Diags(new DiagnosticIDs(), new DiagnosticOptions,
                           new IgnoringDiagConsumer());

    Diags.setSuppressAfterFatalError(Suppress);
    Diags.Report(diag::err_cannot_open_file) << "file"
                                             << "error";
    Diags.Report(diag::warn_mt_message) << "warning";
    EXPECT_TRUE(Diags.hasErrorOcurred());
    EXPECT_TRUE(Diags.hasFatalErrorOcurred());
    EXPECT_TRUE(Diags.hasUncompilableErrorOcurred());
    EXPECT_TRUE(Diags.hasUnrecoverableErrorOcurred());

    EXPECT_EQ(Diags.getNumWarnings(), Suppress ? 0u : 1u);
  }
}

TEST(DiagnosticTest, diagnosticError) {
  DiagnosticsEngine Diags(new DiagnosticIDs(), new DiagnosticOptions,
                         new IgnoringDiagConsumer());
  PartialDiagnostic::StorageAllocator Alloc;
  llvm::Expected<std::pair<int, int>> Value = DiagnosticError::create(
      SourceLocation(), PartialDiagnostic(diag::err_cannot_open_file, Alloc)
                            << "file"
                            << "error");
  ASSERT_TRUE(!Value);
  llvm::Error Err = Value.takeError();
  Optional<PartialDiagnosticAt> ErrDiag = DiagnosticError::take(Err);
  llvm::cantFail(std::move(Err));
  ASSERT_FALSE(!ErrDiag);
  EXPECT_EQ(ErrDiag->first, SourceLocation());
  EXPECT_EQ(ErrDiag->second.getDiagID(), diag::err_cannot_open_file);
  Value = std::make_pair(20, 1);
  ASSERT_FALSE(!Value);
  EXPECT_EQ(*Value, std::make_pair(20, 1));
  EXPECT_EQ(Value->first, 20);
}

} // namespace
