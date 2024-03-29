//===- JsonSupport.h - JSON Output Utilities --------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_BASIC_JSONSUPPORT_H
#define LLVM_LATINO_BASIC_JSONSUPPORT_H

#include "latino/Basic/LLVM.h"
#include "latino/Basic/SourceManager.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include <iterator>

namespace latino {

inline raw_ostream &Indent(raw_ostream &Out, const unsigned int Space,
                           bool IsDot) {
  for (unsigned int I = 0; I < Space * 2; ++I)
    Out << (IsDot ? "&nbsp;" : " ");
  return Out;
}

inline std::string JsonFormat(StringRef RawSR, bool AddQuotes) {
  if (RawSR.empty())
    return "null";

  // Trim special characters.
  std::string Str = RawSR.trim().str();
  size_t Pos = 0;

  // Escape backslashes.
  while (true) {
    Pos = Str.find('\\', Pos);
    if (Pos == std::string::npos)
      break;

    // Prevent bad conversions.
    size_t TempPos = (Pos != 0) ? Pos - 1 : 0;

    // See whether the current backslash is not escaped.
    if (TempPos != Str.find("\\\\", Pos)) {
      Str.insert(Pos, "\\");
      ++Pos; // As we insert the backslash move plus one.
    }

    ++Pos;
  }

  // Escape double quotes.
  Pos = 0;
  while (true) {
    Pos = Str.find('\"', Pos);
    if (Pos == std::string::npos)
      break;

    // Prevent bad conversions.
    size_t TempPos = (Pos != 0) ? Pos - 1 : 0;

    // See whether the current double quote is not escaped.
    if (TempPos != Str.find("\\\"", Pos)) {
      Str.insert(Pos, "\\");
      ++Pos; // As we insert the escape-character move plus one.
    }

    ++Pos;
  }

  // Remove new-lines.
  Str.erase(std::remove(Str.begin(), Str.end(), '\n'), Str.end());

  if (!AddQuotes)
    return Str;

  return '\"' + Str + '\"';
}

inline void printSourceLocationAsJson(raw_ostream &Out, SourceLocation Loc,
                                      const SourceManager &SM,
                                      bool AddBraces = true) {
  // Mostly copy-pasted from SourceLocation::print.
  if (!Loc.isValid()) {
    Out << "null";
    return;
  }

  if (Loc.isFileID()) {
    PresumedLoc PLoc = SM.getPresumedLoc(Loc);

    if (PLoc.isInvalid()) {
      Out << "null";
      return;
    }
    // The macro expansion and spelling pos is identical for file locs.
    if (AddBraces)
      Out << "{ ";
    std::string filename(PLoc.getFilename());
#ifdef _WIN32
    // Remove forbidden Windows path characters
    auto RemoveIt =
        std::remove_if(filename.begin(), filename.end(), [](auto Char) {
          static const char ForbiddenChars[] = "<>*?\"|";
          return std::find(std::begin(ForbiddenChars), std::end(ForbiddenChars),
                           Char) != std::end(ForbiddenChars);
        });
    filename.erase(RemoveIt, filename.end());
    // Handle windows-specific path delimiters.
    std::replace(filename.begin(), filename.end(), '\\', '/');
#endif
    Out << "\"line\": " << PLoc.getLine()
        << ", \"column\": " << PLoc.getColumn()
        << ", \"file\": \"" << filename << "\"";
    if (AddBraces)
      Out << " }";
    return;
  }

  // We want 'location: { ..., spelling: { ... }}' but not
  // 'location: { ... }, spelling: { ... }', hence the dance
  // with braces.
  Out << "{ ";
  printSourceLocationAsJson(Out, SM.getExpansionLoc(Loc), SM, false);
  Out << ", \"spelling\": ";
  printSourceLocationAsJson(Out, SM.getSpellingLoc(Loc), SM, true);
  Out << " }";
}
} // namespace latino

#endif // LLVM_LATINO_BASIC_JSONSUPPORT_H
