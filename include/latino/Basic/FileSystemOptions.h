#ifndef LATINO_FILESYSTEMOPTIONS_H
#define LATINO_FILESYSTEMOPTIONS_H

#include <string>

namespace latino {

/// Keeps track of options that affect how file operations are performed.
class FileSystemOptions {
public:
  /// If set, paths are resolved as if the working directory was
  /// set to the value of WorkingDir.
  std::string WorkingDir;
}; /* FileSystemOptions */

} /* namespace latino */

#endif /* LATINO_FILESYSTEMOPTIONS_H */
