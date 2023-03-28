latino_tablegen(Attrs.inc -gen-clang-attr-classes
  SOURCE ../Basic/Attr.td
  TARGET LatinoAttrClasses)

latino_tablegen(AttrImpl.inc -gen-clang-attr-impl
  SOURCE ../Basic/Attr.td
  TARGET LatinoAttrImpl)

latino_tablegen(AttrTextNodeDump.inc -gen-clang-attr-text-node-dump
  SOURCE ../Basic/Attr.td
  TARGET LatinoAttrTextDump)

latino_tablegen(AttrNodeTraverse.inc -gen-clang-attr-node-traverse
  SOURCE ../Basic/Attr.td
  TARGET LatinoAttrTraverse)

latino_tablegen(AttrVisitor.inc -gen-clang-attr-ast-visitor
  SOURCE ../Basic/Attr.td
  TARGET LatinoAttrVisitor)

latino_tablegen(StmtNodes.inc -gen-clang-stmt-nodes
  SOURCE ../Basic/StmtNodes.td
  TARGET LatinoStmtNodes)

latino_tablegen(DeclNodes.inc -gen-clang-decl-nodes
  SOURCE ../Basic/DeclNodes.td
  TARGET LatinoDeclNodes)

latino_tablegen(TypeNodes.inc -gen-clang-type-nodes
  SOURCE ../Basic/TypeNodes.td
  TARGET LatinoTypeNodes)

latino_tablegen(AbstractBasicReader.inc -gen-clang-basic-reader
  SOURCE PropertiesBase.td
  TARGET LatinoAbstractBasicReader)

latino_tablegen(AbstractBasicWriter.inc -gen-clang-basic-writer
  SOURCE PropertiesBase.td
  TARGET LatinoAbstractBasicWriter)

latino_tablegen(AbstractTypeReader.inc -gen-clang-type-reader
  SOURCE TypeProperties.td
  TARGET LatinoAbstractTypeReader)

latino_tablegen(AbstractTypeWriter.inc -gen-clang-type-writer
  SOURCE TypeProperties.td
  TARGET LatinoAbstractTypeWriter)

latino_tablegen(CommentNodes.inc -gen-clang-comment-nodes
  SOURCE ../Basic/CommentNodes.td
  TARGET LatinoCommentNodes)

latino_tablegen(CommentHTMLTags.inc -gen-clang-comment-html-tags
  SOURCE CommentHTMLTags.td
  TARGET LatinoCommentHTMLTags)

latino_tablegen(CommentHTMLTagsProperties.inc -gen-clang-comment-html-tags-properties
  SOURCE CommentHTMLTags.td
  TARGET LatinoCommentHTMLTagsProperties)

latino_tablegen(CommentHTMLNamedCharacterReferences.inc -gen-clang-comment-html-named-character-references
  SOURCE CommentHTMLNamedCharacterReferences.td
  TARGET LatinoCommentHTMLNamedCharacterReferences)

latino_tablegen(CommentCommandInfo.inc -gen-clang-comment-command-info
  SOURCE CommentCommands.td
  TARGET LatinoCommentCommandInfo)

latino_tablegen(CommentCommandList.inc -gen-clang-comment-command-list
  SOURCE CommentCommands.td
  TARGET LatinoCommentCommandList)

latino_tablegen(StmtDataCollectors.inc -gen-clang-data-collectors
  SOURCE StmtDataCollectors.td
  TARGET StmtDataCollectors)
