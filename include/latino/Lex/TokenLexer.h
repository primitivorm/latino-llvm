//===- TokenLexer.h - Lex from a token buffer -------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the TokenLexer interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_LEX_TOKENLEXER_H
#define LLVM_LATINO_LEX_TOKENLEXER_H

#include "clang/Basic/SourceLocation.h"
#include "llvm/ADT/ArrayRef.h"

namespace latino {
    class Preprocessor;
    
    /// TokenLexer - This implements a lexer that returns tokens from a macro body
/// or token stream instead of lexing from a character buffer.  This is used for
/// macro expansion and _Pragma handling, for example.
class TokenLexer {
    friend class Preprocessor;
};
} // namespace latino

#endif // LLVM_LATINO_LEX_TOKENLEXER_H
