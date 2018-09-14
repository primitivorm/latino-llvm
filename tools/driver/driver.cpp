#include "latino/Basic/DiagnosticOptions.h"
#include "latino/Lex/Lexer.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Option/Option.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Regex.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/StringSaver.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>
#include <set>
#include <system_error>

#include <iostream>

using namespace std;
using namespace latino;
using namespace llvm;

int main(int argc_, const char **argv_) {
  llvm::InitLLVM X(argc_, argv_);
  /*llvm::sys::PrintStackTraceOnErrorSignal(argv_[0]);
  llvm::PrettyStackTraceProgram X(argc_, argv_);
  llvm::llvm_shutdown_obj Y;
  if (llvm::sys::Process::FixupStandardFileDescriptors())
    return 1;*/
  SmallVector<const char *, 256> argv(argv_, argv_ + argc_);

  if (llvm::sys::Process::FixupStandardFileDescriptors())
    return 1;
  llvm::InitializeAllTargets();

  //SmallVector<const char *, 256> argv;
  llvm::SpecificBumpPtrAllocator<char> ArgAllocator;
  /*
  std::error_code EC;
  auto Out = llvm::make_unique<raw_fd_ostream>(
      Opts.OutputPath, EC, (Binary ? sys::fs::F_None : sys::fs::F_Text));
  if (EC) {
    llvm::errs() << "error: couldn't get arguments: " << EC.message() << '\n';
  }
  */
  //

  cout << "Hello world!" << endl;
  return 0;
}
