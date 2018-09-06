#ifndef LATINO_BASIC_TARGETOPTIONS_H
#define LATINO_BASIC_TARGETOPTIONS_H

#include "llvm/Target/TargetOptions.h"
#include <string>
#include <vector>

namespace latino {
/// Options for controlling the target.
class TargetOptions {
public:
  /// The name of the target triple to compile for.
  std::string Triple;

  /// When compiling for the device side, contains the triple used to compile
  /// for the host.
  std::string HostTriple;

  /// If given, the name of the target CPU to generate code for.
  std::string CPU;

  /// If given, the unit to use for floating point math.
  std::string FPMath;

  /// If given, the name of the target ABI to use.
  std::string ABI;

  /// The EABI version to use
  llvm::EABI EABIVersion;

  /// If given, the version string of the linker in use.
  std::string LinkerVersion;

  /// The list of target specific features to enable or disable, as written on
  /// the command line.
  std::vector<std::string> FeaturesAsWritten;

  /// The list of target specific features to enable or disable -- this should
  /// be a list of strings starting with by '+' or '-'.
  std::vector<std::string> Features;

  /// Supported OpenCL extensions and optional core features.
  // OpenCLOptions SupportedOpenCLOptions;

  /// The list of OpenCL extensions to enable or disable, as written on
  /// the command line.
  // std::vector<std::string> OpenCLExtensionsAsWritten;

  /// If given, enables support for __int128_t and __uint128_t types.
  bool ForceEnableInt128 = false;

  /// \brief If enabled, use 32-bit pointers for accessing const/local/shared
  /// address space.
  bool NVPTXUseShortPointers = false;
}; /* TargetOptions */
} /* namespace latino */

#endif /* LATINO_BASIC_TARGETOPTIONS_H */
