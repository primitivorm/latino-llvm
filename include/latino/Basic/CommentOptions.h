#ifndef LATINO_BASIC_COMMENTOPTIONS_H
#define LATINO_BASIC_COMMENTOPTIONS_H

#include <string>
#include <vector>

namespace latino {

/// Options for controlling comment parsing.
struct CommentOptions {
  using BlockCommandNamesTy = std::vector<std::string>;

  /// Command names to treat as block commands in comments.
  /// Should not include the leading backslash.
  BlockCommandNamesTy BlockCommandNames;

  /// Treat ordinary comments as documentation comments.
  bool ParseAllComments = false;
  CommentOptions() = default;
};
} // namespace latino

#endif /* LATINO_BASIC_COMMENTOPTIONS_H */
