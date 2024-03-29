//===-- TestModuleFileExtension.h - Module Extension Tester -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LATINO_FRONTEND_TESTMODULEFILEEXTENSION_H
#define LLVM_LATINO_FRONTEND_TESTMODULEFILEEXTENSION_H

#include "latino/Serialization/ModuleFileExtension.h"
#include "latino/Basic/LLVM.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Bitstream/BitstreamReader.h"
#include <string>

namespace latino {

/// A module file extension used for testing purposes.
class TestModuleFileExtension : public ModuleFileExtension {
  std::string BlockName;
  unsigned MajorVersion;
  unsigned MinorVersion;
  bool Hashed;
  std::string UserInfo;

  class Writer : public ModuleFileExtensionWriter {
  public:
    Writer(ModuleFileExtension *Ext) : ModuleFileExtensionWriter(Ext) { }
    ~Writer() override;

    void writeExtensionContents(Sema &SemaRef,
                                llvm::BitstreamWriter &Stream) override;
  };

  class Reader : public ModuleFileExtensionReader {
    llvm::BitstreamCursor Stream;

  public:
    ~Reader() override;

    Reader(ModuleFileExtension *Ext, const llvm::BitstreamCursor &InStream);
  };

public:
  TestModuleFileExtension(StringRef BlockName,
                          unsigned MajorVersion,
                          unsigned MinorVersion,
                          bool Hashed,
                          StringRef UserInfo)
    : BlockName(BlockName),
      MajorVersion(MajorVersion), MinorVersion(MinorVersion),
      Hashed(Hashed), UserInfo(UserInfo) { }
  ~TestModuleFileExtension() override;

  ModuleFileExtensionMetadata getExtensionMetadata() const override;

  llvm::hash_code hashExtension(llvm::hash_code Code) const override;

  std::unique_ptr<ModuleFileExtensionWriter>
  createExtensionWriter(ASTWriter &Writer) override;

  std::unique_ptr<ModuleFileExtensionReader>
  createExtensionReader(const ModuleFileExtensionMetadata &Metadata,
                        ASTReader &Reader, serialization::ModuleFile &Mod,
                        const llvm::BitstreamCursor &Stream) override;
};

} // end namespace latino

#endif // LLVM_LATINO_FRONTEND_TESTMODULEFILEEXTENSION_H
