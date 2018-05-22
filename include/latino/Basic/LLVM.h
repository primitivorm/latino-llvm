#ifndef LATINO_BASIC_LLVM_H
#define LATINO_BASIC_LLVM_H

#include "llvm/ADT/None.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/MemoryBuffer.h"

namespace llvm {
class StringRef;
class Twine;
template <typename T> class ArrayRef;
template <typename T> class MutableArrayRef;
template <typename T> class OwningArrayRef;
template <unsigned InternalLen> class SmallString;
template <typename T, unsigned N> class SmallVector;
template <typename T> class SmallVectorImpl;
template <typename T> class Optional;
template <class T> class Expected;

template <typename T> struct SaveAndRestore;

// Reference counting.
template <typename T> class IntrusiveRefCntPtr;
template <typename T> struct IntrusiveRefCntPtrInfo;
template <class Derived> class RefCountedBase;

class raw_ostream;
class raw_pwrite_stream;
} // namespace llvm

namespace latino {
// Casting operators.
using llvm::cast;
using llvm::cast_or_null;
using llvm::dyn_cast;
using llvm::dyn_cast_or_null;
using llvm::isa;

// ADT's.
using llvm::ArrayRef;
using llvm::MutableArrayRef;
using llvm::None;
using llvm::Optional;
using llvm::OwningArrayRef;
using llvm::SaveAndRestore;
using llvm::SmallString;
using llvm::SmallVector;
using llvm::SmallVectorImpl;
using llvm::StringRef;
using llvm::StringSwitch;
using llvm::Twine;

// Error handling.
using llvm::Expected;

// Reference counting.
using llvm::IntrusiveRefCntPtr;
using llvm::IntrusiveRefCntPtrInfo;
using llvm::RefCountedBase;

using llvm::raw_ostream;
using llvm::raw_pwrite_stream;

} // namespace latino

#endif /* LATINO_BASIC_LLVM_H */
