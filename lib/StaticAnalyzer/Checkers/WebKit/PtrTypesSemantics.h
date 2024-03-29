//=======- PtrTypesSemantics.cpp ---------------------------------*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_ANALYZER_WEBKIT_PTRTYPESEMANTICS_H
#define LLVM_LATINO_ANALYZER_WEBKIT_PTRTYPESEMANTICS_H

namespace latino {
class CXXBaseSpecifier;
class CXXMethodDecl;
class CXXRecordDecl;
class Expr;
class FunctionDecl;
class Type;

// Ref-countability of a type is implicitly defined by Ref<T> and RefPtr<T>
// implementation. It can be modeled as: type T having public methods ref() and
// deref()

// In WebKit there are two ref-counted templated smart pointers: RefPtr<T> and
// Ref<T>.

/// \returns CXXRecordDecl of the base if the type is ref-countable, nullptr if
/// not.
const latino::CXXRecordDecl *isRefCountable(const latino::CXXBaseSpecifier *Base);

/// \returns true if \p Class is ref-countable, false if not.
/// Asserts that \p Class IS a definition.
bool isRefCountable(const latino::CXXRecordDecl *Class);

/// \returns true if \p Class is ref-counted, false if not.
bool isRefCounted(const latino::CXXRecordDecl *Class);

/// \returns true if \p Class is ref-countable AND not ref-counted, false if
/// not. Asserts that \p Class IS a definition.
bool isUncounted(const latino::CXXRecordDecl *Class);

/// \returns true if \p T is either a raw pointer or reference to an uncounted
/// class, false if not.
bool isUncountedPtr(const latino::Type *T);

/// \returns true if \p F creates ref-countable object from uncounted parameter,
/// false if not.
bool isCtorOfRefCounted(const latino::FunctionDecl *F);

/// \returns true if \p M is getter of a ref-counted class, false if not.
bool isGetterOfRefCounted(const latino::CXXMethodDecl *Method);

/// \returns true if \p F is a conversion between ref-countable or ref-counted
/// pointer types.
bool isPtrConversion(const FunctionDecl *F);

} // namespace latino

#endif
