//===- GuessTargetAndModeCompilationDatabase.cpp --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "latino/Tooling/CompilationDatabase.h"
#include "latino/Tooling/Tooling.h"
#include <memory>

namespace latino {
namespace tooling {

namespace {
class TargetAndModeAdderDatabase : public CompilationDatabase {
public:
  TargetAndModeAdderDatabase(std::unique_ptr<CompilationDatabase> Base)
      : Base(std::move(Base)) {
    assert(this->Base != nullptr);
  }

  std::vector<std::string> getAllFiles() const override {
    return Base->getAllFiles();
  }

  std::vector<CompileCommand> getAllCompileCommands() const override {
    return addTargetAndMode(Base->getAllCompileCommands());
  }

  std::vector<CompileCommand>
  getCompileCommands(StringRef FilePath) const override {
    return addTargetAndMode(Base->getCompileCommands(FilePath));
  }

private:
  std::vector<CompileCommand>
  addTargetAndMode(std::vector<CompileCommand> Cmds) const {
    for (auto &Cmd : Cmds) {
      if (Cmd.CommandLine.empty())
        continue;
      addTargetAndModeForProgramName(Cmd.CommandLine, Cmd.CommandLine.front());
    }
    return Cmds;
  }
  std::unique_ptr<CompilationDatabase> Base;
};
} // namespace

std::unique_ptr<CompilationDatabase>
inferTargetAndDriverMode(std::unique_ptr<CompilationDatabase> Base) {
  return std::make_unique<TargetAndModeAdderDatabase>(std::move(Base));
}

} // namespace tooling
} // namespace latino
