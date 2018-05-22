#include "latino/Lex/Lexer.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/StringSaver.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"

#include <iostream>

using namespace std;
using namespace latino;
using namespace llvm;

int main(int argc_, const char **argv_) {
  llvm::sys::PrintStackTraceOnErrorSignal(argv_[0]);
  llvm::PrettyStackTraceProgram X(argc_, argv_);
  llvm::llvm_shutdown_obj Y;
  if (llvm::sys::Process::FixupStandardFileDescriptors())
    return 1;
  SmallVector<const char *, 256> argv;
  llvm::SpecificBumpPtrAllocator<char> ArgAllocator;
  /*
  std::error_code EC;
  auto Out = llvm::make_unique<raw_fd_ostream>(
      Opts.OutputPath, EC, (Binary ? sys::fs::F_None : sys::fs::F_Text));
  if (EC) {
    llvm::errs() << "error: couldn't get arguments: " << EC.message() << '\n';
  }
  */
  llvm::InitializeAllTargets();

  cout << "Hello world!" << endl;
  return 0;
}
