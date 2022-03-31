//===- CodeCompleteConsumer.h - Code Completion Interface -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the CodeCompleteConsumer class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_SEMA_CODECOMPLETECONSUMER_H
#define LLVM_LATINO_SEMA_CODECOMPLETECONSUMER_H

namespace latino {
/// Abstract interface for a consumer of code-completion
/// information.
class CodeCompleteConsumer {};

} // namespace latino

#endif