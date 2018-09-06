//===----------------------------------------------------------------------===//
///
/// \file
/// Defines the latino::TargetInfo interface.
///
//===----------------------------------------------------------------------===//

#ifndef LATINO_BASIC_TARGETINFO_H
#define LATINO_BASIC_TARGETINFO_H

//#include "latino/Basic/AddressSpaces.h"
#include "latino/Basic/LLVM.h"
#include "latino/Basic/Specifiers.h"
//#include "latino/Basic/TargetCXXABI.h"
#include "latino/Basic/TargetOptions.h"

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Triple.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Support/DataTypes.h"
//#include "llvm/Support/VersionTuple.h"

#include <cassert>
#include <string>
#include <vector>

namespace llvm {
struct fltSemantics;
}

namespace latino {
class DiagnosticsEngine;
class LangOptions;
class CodeGenOptions;
// class MacroBuilder;
// class QualType;
class SourceLocation;
class SourceManager;

namespace Builtin {
struct Info;
}

/// Exposes information about the current target.
///
class TargetInfo : public RefCountedBase<TargetInfo> {
  std::shared_ptr<TargetOptions> TargetOpts;
  llvm::Triple Triple;

protected:
  // Target values set by the ctor of the actual target implementation.  Default
  // values are specified by the TargetInfo constructor.
  bool BigEndian;
  bool TLSSupported;
  bool VLASupported;
  bool NoAsmVariants;    // True if {|} are normal characters.
  bool HasLegalHalfType; // True if the backend supports operations on the half
                         // LLVM IR type.
  bool HasFloat128;
  unsigned char PointerWidth, PointerAlign;
  unsigned char BoolWidth, BoolAlign;
  unsigned char IntWidth, IntAlign;
  unsigned char HalfWidth, HalfAlign;
  unsigned char FloatWidth, FloatAlign;
  unsigned char DoubleWidth, DoubleAlign;
  unsigned char LongDoubleWidth, LongDoubleAlign, Float128Align;
  unsigned char LargeArrayMinWidth, LargeArrayAlign;
  unsigned char LongWidth, LongAlign;
  unsigned char LongLongWidth, LongLongAlign;
  unsigned char ShortAccumWidth, ShortAccumAlign;
  unsigned char AccumWidth, AccumAlign;
  unsigned char LongAccumWidth, LongAccumAlign;
  unsigned char ShortFractWidth, ShortFractAlign;
  unsigned char FractWidth, FractAlign;
  unsigned char LongFractWidth, LongFractAlign;
  unsigned char DefaultAlignForAttributeAligned;
  unsigned char MinGlobalAlign;
  unsigned char MaxAtomicPromoteWidth, MaxAtomicInlineWidth;
  unsigned short MaxVectorAlign;
  unsigned short MaxTLSAlign;
  unsigned short SimdDefaultAlign;
  unsigned short NewAlign;
  std::unique_ptr<llvm::DataLayout> DataLayout;
  const char *MCountName;
  const llvm::fltSemantics *HalfFormat, *FloatFormat, *DoubleFormat,
      *LongDoubleFormat, *Float128Format;
  unsigned char RegParmMax, SSERegParmMax;
  // TargetCXXABI TheCXXABI;
  // const LangASMap *AddrSpaceMap;

  mutable StringRef PlatformName;
  // mutable VersionTuple PlatformMinVersion;

  unsigned HasAlignMc68kSupport : 1;
  unsigned RealTypeUsesObjCFPRet : 3;
  unsigned ComplexLongDoubleUsesFP2Ret : 1;

  unsigned HasBuiltinMSVaList : 1;

  unsigned IsRenderScriptTarget : 1;

  // TargetInfo Constructor.  Default initializes all fields.
  TargetInfo(const llvm::Triple &T);

  void resetDataLayout(StringRef DL) {
    DataLayout.reset(new llvm::DataLayout(DL));
  }

public:
  /// Construct a target for the given options.
  ///
  /// \param Opts - The options to use to initialize the target. The target may
  /// modify the options to canonicalize the target feature information to match
  /// what the backend expects.
  static TargetInfo *
  CreateTargetInfo(DiagnosticsEngine &Diags,
                   const std::shared_ptr<TargetOptions> &Opts);

  virtual ~TargetInfo();

  /// Retrieve the target options.
  TargetOptions &getTargetOpts() const {
    assert(TargetOpts && "Missing target options");
    return *TargetOpts;
  }

  ///===---- Target Data Type Query Methods -------------------------------===//
  enum IntType {
    NoInt = 0,
    SignedChar,
    UnsignedChar,
    SignedShort,
    UnsignedShort,
    SignedInt,
    UnsignedInt,
    SignedLong,
    UnsignedLong,
    SignedLongLong,
    UnsignedLongLong
  };

  enum RealType { NoFloat = 255, Float = 0, Double, LongDouble, Float128 };

  /// The different kinds of __builtin_va_list types defined by
  /// the target implementation.
  enum BuiltinVaListKind {
    /// typedef char* __builtin_va_list;
    CharPtrBuiltinVaList = 0,

    /// typedef void* __builtin_va_list;
    VoidPtrBuiltinVaList,

    /// __builtin_va_list as defined by the AArch64 ABI
    /// http://infocenter.arm.com/help/topic/com.arm.doc.ihi0055a/IHI0055A_aapcs64.pdf
    AArch64ABIBuiltinVaList,

    /// __builtin_va_list as defined by the PNaCl ABI:
    /// http://www.chromium.org/nativeclient/pnacl/bitcode-abi#TOC-Machine-Types
    PNaClABIBuiltinVaList,

    /// __builtin_va_list as defined by the Power ABI:
    /// https://www.power.org
    ///        /resources/downloads/Power-Arch-32-bit-ABI-supp-1.0-Embedded.pdf
    PowerABIBuiltinVaList,

    /// __builtin_va_list as defined by the x86-64 ABI:
    /// http://refspecs.linuxbase.org/elf/x86_64-abi-0.21.pdf
    X86_64ABIBuiltinVaList,

    /// __builtin_va_list as defined by ARM AAPCS ABI
    /// http://infocenter.arm.com
    //        /help/topic/com.arm.doc.ihi0042d/IHI0042D_aapcs.pdf
    AAPCSABIBuiltinVaList,

    // typedef struct __va_list_tag
    //   {
    //     long __gpr;
    //     long __fpr;
    //     void *__overflow_arg_area;
    //     void *__reg_save_area;
    //   } va_list[1];
    SystemZBuiltinVaList
  };

protected:
  IntType SizeType, IntMaxType, PtrDiffType, IntPtrType, WCharType, WIntType,
      Char16Type, Char32Type, Int64Type, SigAtomicType, ProcessIDType;

}; /* TargetInfo */

} /* namespace latino */

#endif /* LATINO_BASIC_TARGETINFO_H */
