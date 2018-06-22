#ifndef LATINO_BASIC_COMMENTOPTIONS_H
#define LATINO_BASIC_COMMENTOPTIONS_H

#include <string>
#include <vector>

namespace latino {
struct CommentOptions {
  using BlockCommandNamesTy = std::vector<std::string>;
  BlockCommandNamesTy BlockCommandNames;
  bool ParseAllComments = false;
  CommentOptions() = default;
};
} // namespace latino

#endif
