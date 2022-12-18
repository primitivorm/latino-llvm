//===- CommentOptions.h - Options for parsing comments ----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Defines the latino::CommentOptions interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_BASIC_COMMENTOPTIONS_H
#define LLVM_LATINO_BASIC_COMMENTOPTIONS_H

#include <string>
#include <vector>

namespace latino {

/// Options for controlling comment parsing.
struct CommentOptions {
  using BlockCommandNamesTy = std::vector<std::string>;

  /// Command names to treat as block commands in comments.
  /// Should not include the leading backslash.
  BlockCommandNamesTy BlockCommandNames;

  /// Treat ordinary comments as documentation comments.
  bool ParseAllComments = false;

  CommentOptions() = default;
};

} // namespace latino

#endif // LLVM_LATINO_BASIC_COMMENTOPTIONS_H
