//===----------------------------------------------------------------------===//
///
/// \file
/// Defines the latino::Visibility enumeration and various utility
/// functions.
///
//===----------------------------------------------------------------------===//
#ifndef LATINO_BASIC_VISIBILITY_H
#define LATINO_BASIC_VISIBILITY_H

#include <cassert>
#include <cstdint>

namespace latino {
/// Describes the different kinds of visibility that a declaration
/// may have.
///
/// Visibility determines how a declaration interacts with the dynamic
/// linker.  It may also affect whether the symbol can be found by runtime
/// symbol lookup APIs.
///
/// Visibility is not described in any language standard and
/// (nonetheless) sometimes has odd behavior.  Not all platforms
/// support all visibility kinds.
enum Visibility {
  /// Objects with "hidden" visibility are not seen by the dynamic
  /// linker.
  HiddenVisibility,

  /// Objects with "protected" visibility are seen by the dynamic
  /// linker but always dynamically resolve to an object within this
  /// shared object.
  ProtectedVisibility,

  /// Objects with "default" visibility are seen by the dynamic linker
  /// and act like normal objects.
  DefaultVisibility
};

} // namespace latino

#endif /* LATINO_BASIC_VISIBILITY_H */
