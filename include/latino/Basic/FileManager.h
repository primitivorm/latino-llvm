#ifndef LATINO_BASIC_FILEMANAGER_H
#define LATINO_BASIC_FILEMANAGER_H

#include "llvm/ADT/IntrusiveRefCntPtr.h"

using namespace llvm;

namespace latino {
class FileEntry {}; /* FileEntry */

class FileManager : public RefCountedBase<FileManager> {}; /* FileManager */
} /* namespace latino */

#endif
