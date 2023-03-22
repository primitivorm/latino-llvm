//===--- RefactoringOptions.h - Clang refactoring library -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_TOOLING_REFACTOR_REFACTORING_OPTIONS_H
#define LLVM_LATINO_TOOLING_REFACTOR_REFACTORING_OPTIONS_H

#include "latino/Basic/LLVM.h"
#include "latino/Tooling/Refactoring/RefactoringActionRuleRequirements.h"
#include "latino/Tooling/Refactoring/RefactoringOption.h"
#include "latino/Tooling/Refactoring/RefactoringOptionVisitor.h"
#include "llvm/Support/Error.h"
#include <type_traits>

namespace latino {
namespace tooling {

/// A refactoring option that stores a value of type \c T.
template <typename T,
          typename = std::enable_if_t<traits::IsValidOptionType<T>::value>>
class OptionalRefactoringOption : public RefactoringOption {
public:
  void passToVisitor(RefactoringOptionVisitor &Visitor) final override {
    Visitor.visit(*this, Value);
  }

  bool isRequired() const override { return false; }

  using ValueType = Optional<T>;

  const ValueType &getValue() const { return Value; }

protected:
  Optional<T> Value;
};

/// A required refactoring option that stores a value of type \c T.
template <typename T,
          typename = std::enable_if_t<traits::IsValidOptionType<T>::value>>
class RequiredRefactoringOption : public OptionalRefactoringOption<T> {
public:
  using ValueType = T;

  const ValueType &getValue() const {
    return *OptionalRefactoringOption<T>::Value;
  }
  bool isRequired() const final override { return true; }
};

} // end namespace tooling
} // end namespace latino

#endif // LLVM_LATINO_TOOLING_REFACTOR_REFACTORING_OPTIONS_H
