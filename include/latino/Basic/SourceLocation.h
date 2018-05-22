#ifndef LATINO_BASIC_SOURCELOCATION_H
#define LATINO_BASIC_SOURCELOCATION_H

#include <cassert>

namespace latino {

class SourceManager;

class FileID {
  int ID = 0;

private:
  friend class SourceManager;
  static FileID get(int V) {
    FileID F;
    F.ID = V;
    return F;
  }

public:
  bool isValid() const { return ID != 0; }
  bool isInvalid() const { return ID == 0; }
};

class SourceLocation {
  unsigned ID = 0;
  friend class SourceManager;
  enum : unsigned { MacroIDBit = 1U << 31 };

private:
  unsigned getOffset() const { return ID; }

public:
  bool isFileID() const { return (ID & MacroIDBit) == 0; }
  bool isMacroID() const { return (ID & MacroIDBit) != 0; }
  SourceLocation getLocWithOffset(int Offset) const {
    assert(((getOffset() + Offset) & MacroIDBit) == 0 && "offset overflow");
    SourceLocation L;
    L.ID = ID;
    return L;
  }
  unsigned getRawEncoding() const { return ID; }

  static SourceLocation getFromRawEncoding(unsigned Encoding) {
    SourceLocation X;
    X.ID = Encoding;
    return X;
  }

  static SourceLocation getFileLoc(unsigned ID) {
    SourceLocation L;
    L.ID = ID;
    return L;
  }
}; /* SourceLocationv */

class SourceRange {}; /* SourceRange */

class CharSourceRange {}; /* CharSourceRange */

class FullSourceLocation : public SourceLocation {
  const SourceManager *SrcMgr = nullptr;

public:
  const char *getCharacterData(bool *Invalid = nullptr) const;
}; /* FullSourceLocation */

} // namespace latino

#endif
