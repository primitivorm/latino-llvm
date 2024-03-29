//===--- CommentToXML.h - Convert comments to XML representation ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LATINO_INDEX_COMMENTTOXML_H
#define LLVM_LATINO_INDEX_COMMENTTOXML_H

#include "latino/Basic/LLVM.h"
#include <memory>

namespace latino {
class ASTContext;

namespace comments {
class FullComment;
class HTMLTagComment;
}

namespace index {
class CommentToXMLConverter {
public:
  CommentToXMLConverter();
  ~CommentToXMLConverter();

  void convertCommentToHTML(const comments::FullComment *FC,
                            SmallVectorImpl<char> &HTML,
                            const ASTContext &Context);

  void convertHTMLTagNodeToText(const comments::HTMLTagComment *HTC,
                                SmallVectorImpl<char> &Text,
                                const ASTContext &Context);

  void convertCommentToXML(const comments::FullComment *FC,
                           SmallVectorImpl<char> &XML,
                           const ASTContext &Context);
};

} // namespace index
} // namespace latino

#endif // LLVM_LATINO_INDEX_COMMENTTOXML_H

