//===--- Hurd.cpp - Hurd ToolChain Implementations --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Hurd.h"
#include "CommonArgs.h"
#include "latino/Config/config.h"
#include "latino/Driver/Driver.h"
#include "latino/Driver/Options.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/VirtualFileSystem.h"

using namespace latino::driver;
using namespace latino::driver::toolchains;
using namespace latino;
using namespace llvm::opt;

using tools::addPathIfExists;

/// Get our best guess at the multiarch triple for a target.
///
/// Debian-based systems are starting to use a multiarch setup where they use
/// a target-triple directory in the library and header search paths.
/// Unfortunately, this triple does not align with the vanilla target triple,
/// so we provide a rough mapping here.
std::string Hurd::getMultiarchTriple(const Driver &D,
                                     const llvm::Triple &TargetTriple,
                                     StringRef SysRoot) const {
  if (TargetTriple.getArch() == llvm::Triple::x86) {
    // We use the existence of '/lib/<triple>' as a directory to detect some
    // common hurd triples that don't quite match the Clang triple for both
    // 32-bit and 64-bit targets. Multiarch fixes its install triples to these
    // regardless of what the actual target triple is.
    if (D.getVFS().exists(SysRoot + "/lib/i386-gnu"))
      return "i386-gnu";
  }

  // For most architectures, just use whatever we have rather than trying to be
  // clever.
  return TargetTriple.str();
}

static StringRef getOSLibDir(const llvm::Triple &Triple, const ArgList &Args) {
  // It happens that only x86 and PPC use the 'lib32' variant of oslibdir, and
  // using that variant while targeting other architectures causes problems
  // because the libraries are laid out in shared system roots that can't cope
  // with a 'lib32' library search path being considered. So we only enable
  // them when we know we may need it.
  //
  // FIXME: This is a bit of a hack. We should really unify this code for
  // reasoning about oslibdir spellings with the lib dir spellings in the
  // GCCInstallationDetector, but that is a more significant refactoring.

  if (Triple.getArch() == llvm::Triple::x86)
    return "lib32";

  return Triple.isArch32Bit() ? "lib" : "lib64";
}

Hurd::Hurd(const Driver &D, const llvm::Triple &Triple, const ArgList &Args)
    : Generic_ELF(D, Triple, Args) {
  GCCInstallation.init(Triple, Args);
  Multilibs = GCCInstallation.getMultilibs();
  SelectedMultilib = GCCInstallation.getMultilib();
  std::string SysRoot = computeSysRoot();
  ToolChain::path_list &PPaths = getProgramPaths();

  Generic_GCC::PushPPaths(PPaths);

  // The selection of paths to try here is designed to match the patterns which
  // the GCC driver itself uses, as this is part of the GCC-compatible driver.
  // This was determined by running GCC in a fake filesystem, creating all
  // possible permutations of these directories, and seeing which ones it added
  // to the link paths.
  path_list &Paths = getFilePaths();

  const std::string OSLibDir = std::string(getOSLibDir(Triple, Args));
  const std::string MultiarchTriple = getMultiarchTriple(D, Triple, SysRoot);

#ifdef ENABLE_LINKER_BUILD_ID
  ExtraOpts.push_back("--build-id");
#endif

  Generic_GCC::AddMultilibPaths(D, SysRoot, OSLibDir, MultiarchTriple, Paths);

  // Similar to the logic for GCC above, if we currently running Clang inside
  // of the requested system root, add its parent library paths to
  // those searched.
  // FIXME: It's not clear whether we should use the driver's installed
  // directory ('Dir' below) or the ResourceDir.
  if (StringRef(D.Dir).startswith(SysRoot)) {
    addPathIfExists(D, D.Dir + "/../lib/" + MultiarchTriple, Paths);
    addPathIfExists(D, D.Dir + "/../" + OSLibDir, Paths);
  }

  addPathIfExists(D, SysRoot + "/lib/" + MultiarchTriple, Paths);
  addPathIfExists(D, SysRoot + "/lib/../" + OSLibDir, Paths);

  addPathIfExists(D, SysRoot + "/usr/lib/" + MultiarchTriple, Paths);
  addPathIfExists(D, SysRoot + "/usr/lib/../" + OSLibDir, Paths);

  Generic_GCC::AddMultiarchPaths(D, SysRoot, OSLibDir, Paths);

  // Similar to the logic for GCC above, if we are currently running Clang
  // inside of the requested system root, add its parent library path to those
  // searched.
  // FIXME: It's not clear whether we should use the driver's installed
  // directory ('Dir' below) or the ResourceDir.
  if (StringRef(D.Dir).startswith(SysRoot))
    addPathIfExists(D, D.Dir + "/../lib", Paths);

  addPathIfExists(D, SysRoot + "/lib", Paths);
  addPathIfExists(D, SysRoot + "/usr/lib", Paths);
}

bool Hurd::HasNativeLLVMSupport() const { return true; }

Tool *Hurd::buildLinker() const { return new tools::gnutools::Linker(*this); }

Tool *Hurd::buildAssembler() const {
  return new tools::gnutools::Assembler(*this);
}

std::string Hurd::getDynamicLinker(const ArgList &Args) const {
  if (getArch() == llvm::Triple::x86)
    return "/lib/ld.so";

  llvm_unreachable("unsupported architecture");
}

void Hurd::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                     ArgStringList &CC1Args) const {
  const Driver &D = getDriver();
  std::string SysRoot = computeSysRoot();

  if (DriverArgs.hasArg(latino::driver::options::OPT_nostdinc))
    return;

  if (!DriverArgs.hasArg(options::OPT_nostdlibinc))
    addSystemInclude(DriverArgs, CC1Args, SysRoot + "/usr/local/include");

  if (!DriverArgs.hasArg(options::OPT_nobuiltininc)) {
    SmallString<128> P(D.ResourceDir);
    llvm::sys::path::append(P, "include");
    addSystemInclude(DriverArgs, CC1Args, P);
  }

  if (DriverArgs.hasArg(options::OPT_nostdlibinc))
    return;

  // Check for configure-time C include directories.
  StringRef CIncludeDirs(C_INCLUDE_DIRS);
  if (CIncludeDirs != "") {
    SmallVector<StringRef, 5> Dirs;
    CIncludeDirs.split(Dirs, ":");
    for (StringRef Dir : Dirs) {
      StringRef Prefix =
          llvm::sys::path::is_absolute(Dir) ? "" : StringRef(SysRoot);
      addExternCSystemInclude(DriverArgs, CC1Args, Prefix + Dir);
    }
    return;
  }

  // Lacking those, try to detect the correct set of system includes for the
  // target triple.

  AddMultilibIncludeArgs(DriverArgs, CC1Args);

  if (getTriple().getArch() == llvm::Triple::x86) {
    std::string Path = SysRoot + "/usr/include/i386-gnu";
    if (D.getVFS().exists(Path))
      addExternCSystemInclude(DriverArgs, CC1Args, Path);
  }

  // Add an include of '/include' directly. This isn't provided by default by
  // system GCCs, but is often used with cross-compiling GCCs, and harmless to
  // add even when Clang is acting as-if it were a system compiler.
  addExternCSystemInclude(DriverArgs, CC1Args, SysRoot + "/include");

  addExternCSystemInclude(DriverArgs, CC1Args, SysRoot + "/usr/include");
}

void Hurd::addExtraOpts(llvm::opt::ArgStringList &CmdArgs) const {
  for (const auto &Opt : ExtraOpts)
    CmdArgs.push_back(Opt.c_str());
}
