//===--- CreateInvocationFromCommandLine.cpp - CompilerInvocation from Args ==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Construct a compiler invocation object for command line driver arguments
//
//===----------------------------------------------------------------------===//

#include "latino/Frontend/Utils.h"
#include "latino/Basic/DiagnosticOptions.h"
#include "latino/Driver/Compilation.h"
#include "latino/Driver/Driver.h"
#include "latino/Driver/Action.h"
#include "latino/Driver/Options.h"
#include "latino/Driver/Tool.h"
#include "latino/Frontend/CompilerInstance.h"
#include "latino/Frontend/FrontendDiagnostic.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/Host.h"
using namespace latino;
using namespace llvm::opt;

std::unique_ptr<CompilerInvocation> latino::createInvocationFromCommandLine(
    ArrayRef<const char *> ArgList, IntrusiveRefCntPtr<DiagnosticsEngine> Diags,
    IntrusiveRefCntPtr<llvm::vfs::FileSystem> VFS, bool ShouldRecoverOnErorrs,
    std::vector<std::string> *CC1Args) {
  if (!Diags.get()) {
    // No diagnostics engine was provided, so create our own diagnostics object
    // with the default options.
    Diags = CompilerInstance::createDiagnostics(new DiagnosticOptions);
  }

  SmallVector<const char *, 16> Args(ArgList.begin(), ArgList.end());

  // FIXME: Find a cleaner way to force the driver into restricted modes.
  Args.push_back("-fsyntax-only");

  // FIXME: We shouldn't have to pass in the path info.
  driver::Driver TheDriver(Args[0], llvm::sys::getDefaultTargetTriple(),
                           *Diags, VFS);

  // Don't check that inputs exist, they may have been remapped.
  TheDriver.setCheckInputsExist(false);

  std::unique_ptr<driver::Compilation> C(TheDriver.BuildCompilation(Args));
  if (!C)
    return nullptr;

  // Just print the cc1 options if -### was present.
  if (C->getArgs().hasArg(driver::options::OPT__HASH_HASH_HASH)) {
    C->getJobs().Print(llvm::errs(), "\n", true);
    return nullptr;
  }

  // We expect to get back exactly one command job, if we didn't something
  // failed. Offload compilation is an exception as it creates multiple jobs. If
  // that's the case, we proceed with the first job. If caller needs a
  // particular job, it should be controlled via options (e.g.
  // --cuda-{host|device}-only for CUDA) passed to the driver.
  const driver::JobList &Jobs = C->getJobs();
  bool OffloadCompilation = false;
  if (Jobs.size() > 1) {
    for (auto &A : C->getActions()){
      // On MacOSX real actions may end up being wrapped in BindArchAction
      if (isa<driver::BindArchAction>(A))
        A = *A->input_begin();
      if (isa<driver::OffloadAction>(A)) {
        OffloadCompilation = true;
        break;
      }
    }
  }
  if (Jobs.size() == 0 || !isa<driver::Command>(*Jobs.begin()) ||
      (Jobs.size() > 1 && !OffloadCompilation)) {
    SmallString<256> Msg;
    llvm::raw_svector_ostream OS(Msg);
    Jobs.Print(OS, "; ", true);
    Diags->Report(diag::err_fe_expected_compiler_job) << OS.str();
    return nullptr;
  }

  const driver::Command &Cmd = cast<driver::Command>(*Jobs.begin());
  if (StringRef(Cmd.getCreator().getName()) != "clang") {
    Diags->Report(diag::err_fe_expected_clang_command);
    return nullptr;
  }

  const ArgStringList &CCArgs = Cmd.getArguments();
  if (CC1Args)
    *CC1Args = {CCArgs.begin(), CCArgs.end()};
  auto CI = std::make_unique<CompilerInvocation>();
  if (!CompilerInvocation::CreateFromArgs(*CI, CCArgs, *Diags, Args[0]) &&
      !ShouldRecoverOnErorrs)
    return nullptr;
  return CI;
}
