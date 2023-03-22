//===--- Opcode.h - Opcodes for the constexpr VM ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Defines all opcodes executed by the VM and emitted by the compiler.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_AST_INTERP_OPCODE_H
#define LLVM_LATINO_AST_INTERP_OPCODE_H

#include <cstdint>

namespace latino {
namespace interp {

enum Opcode : uint32_t {
#define GET_OPCODE_NAMES
#include "Opcodes.inc"
#undef GET_OPCODE_NAMES
};

} // namespace interp
} // namespace latino

#endif
