//===- ArgumentsAdjusters.cpp - Command line arguments adjuster -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains definitions of classes which implement ArgumentsAdjuster
// interface.
//
//===----------------------------------------------------------------------===//

#include "latino/Tooling/ArgumentsAdjusters.h"
#include "latino/Basic/LLVM.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include <cstddef>
#include <vector>

namespace latino {
namespace tooling {

/// Add -fsyntax-only option and drop options that triggers output generation.
ArgumentsAdjuster getClangSyntaxOnlyAdjuster() {
  return [](const CommandLineArguments &Args, StringRef /*unused*/) {
    CommandLineArguments AdjustedArgs;
    bool HasSyntaxOnly = false;
    constexpr llvm::StringRef OutputCommands[] = {
        // FIXME: Add other options that generate output.
        "-save-temps",
        "--save-temps",
    };
    for (size_t i = 0, e = Args.size(); i < e; ++i) {
      StringRef Arg = Args[i];
      // Skip output commands.
      if (llvm::any_of(OutputCommands, [&Arg](llvm::StringRef OutputCommand) {
            return Arg.startswith(OutputCommand);
          }))
        continue;

      if (!Arg.startswith("-fcolor-diagnostics") &&
          !Arg.startswith("-fdiagnostics-color"))
        AdjustedArgs.push_back(Args[i]);
      // If we strip a color option, make sure we strip any preceeding `-Xclang`
      // option as well.
      // FIXME: This should be added to most argument adjusters!
      else if (!AdjustedArgs.empty() && AdjustedArgs.back() == "-Xclang")
        AdjustedArgs.pop_back();

      if (Arg == "-fsyntax-only")
        HasSyntaxOnly = true;
    }
    if (!HasSyntaxOnly)
      AdjustedArgs.push_back("-fsyntax-only");
    return AdjustedArgs;
  };
}

ArgumentsAdjuster getClangStripOutputAdjuster() {
  return [](const CommandLineArguments &Args, StringRef /*unused*/) {
    CommandLineArguments AdjustedArgs;
    for (size_t i = 0, e = Args.size(); i < e; ++i) {
      StringRef Arg = Args[i];
      if (!Arg.startswith("-o"))
        AdjustedArgs.push_back(Args[i]);

      if (Arg == "-o") {
        // Output is specified as -o foo. Skip the next argument too.
        ++i;
      }
      // Else, the output is specified as -ofoo. Just do nothing.
    }
    return AdjustedArgs;
  };
}

ArgumentsAdjuster getClangStripSerializeDiagnosticAdjuster() {
  return [](const CommandLineArguments &Args, StringRef /*unused*/) {
    CommandLineArguments AdjustedArgs;
    for (size_t i = 0, e = Args.size(); i < e; ++i) {
      StringRef Arg = Args[i];
      if (Arg == "--serialize-diagnostics") {
        // Skip the diagnostic output argument.
        ++i;
        continue;
      }
      AdjustedArgs.push_back(Args[i]);
    }
    return AdjustedArgs;
  };
}

ArgumentsAdjuster getClangStripDependencyFileAdjuster() {
  return [](const CommandLineArguments &Args, StringRef /*unused*/) {
    CommandLineArguments AdjustedArgs;
    for (size_t i = 0, e = Args.size(); i < e; ++i) {
      StringRef Arg = Args[i];
      // All dependency-file options begin with -M. These include -MM,
      // -MF, -MG, -MP, -MT, -MQ, -MD, and -MMD.
      if (!Arg.startswith("-M") && !Arg.startswith("/showIncludes") &&
          !Arg.startswith("-showIncludes")) {
        AdjustedArgs.push_back(Args[i]);
        continue;
      }

      if (Arg == "-MF" || Arg == "-MT" || Arg == "-MQ")
        // These flags take an argument: -MX foo. Skip the next argument also.
        ++i;
    }
    return AdjustedArgs;
  };
}

ArgumentsAdjuster getInsertArgumentAdjuster(const CommandLineArguments &Extra,
                                            ArgumentInsertPosition Pos) {
  return [Extra, Pos](const CommandLineArguments &Args, StringRef /*unused*/) {
    CommandLineArguments Return(Args);

    CommandLineArguments::iterator I;
    if (Pos == ArgumentInsertPosition::END) {
      I = Return.end();
    } else {
      I = Return.begin();
      ++I; // To leave the program name in place
    }

    Return.insert(I, Extra.begin(), Extra.end());
    return Return;
  };
}

ArgumentsAdjuster getInsertArgumentAdjuster(const char *Extra,
                                            ArgumentInsertPosition Pos) {
  return getInsertArgumentAdjuster(CommandLineArguments(1, Extra), Pos);
}

ArgumentsAdjuster combineAdjusters(ArgumentsAdjuster First,
                                   ArgumentsAdjuster Second) {
  if (!First)
    return Second;
  if (!Second)
    return First;
  return [First, Second](const CommandLineArguments &Args, StringRef File) {
    return Second(First(Args, File), File);
  };
}

ArgumentsAdjuster getStripPluginsAdjuster() {
  return [](const CommandLineArguments &Args, StringRef /*unused*/) {
    CommandLineArguments AdjustedArgs;
    for (size_t I = 0, E = Args.size(); I != E; I++) {
      // According to https://clang.llvm.org/docs/ClangPlugins.html
      // plugin arguments are in the form:
      // -Xclang {-load, -plugin, -plugin-arg-<plugin-name>, -add-plugin}
      // -Xclang <arbitrary-argument>
      if (I + 4 < E && Args[I] == "-Xclang" &&
          (Args[I + 1] == "-load" || Args[I + 1] == "-plugin" ||
           llvm::StringRef(Args[I + 1]).startswith("-plugin-arg-") ||
           Args[I + 1] == "-add-plugin") &&
          Args[I + 2] == "-Xclang") {
        I += 3;
        continue;
      }
      AdjustedArgs.push_back(Args[I]);
    }
    return AdjustedArgs;
  };
}

} // end namespace tooling
} // end namespace latino
