latino_tablegen(Attrs.inc -gen-clang-attr-classes
  SOURCE ../Basic/Attr.td
  TARGET ClangAttrClasses)

latino_tablegen(AttrImpl.inc -gen-clang-attr-impl
  SOURCE ../Basic/Attr.td
  TARGET ClangAttrImpl)

latino_tablegen(AttrTextNodeDump.inc -gen-clang-attr-text-node-dump
  SOURCE ../Basic/Attr.td
  TARGET ClangAttrTextDump)

latino_tablegen(AttrNodeTraverse.inc -gen-clang-attr-node-traverse
  SOURCE ../Basic/Attr.td
  TARGET ClangAttrTraverse)

latino_tablegen(AttrVisitor.inc -gen-clang-attr-ast-visitor
  SOURCE ../Basic/Attr.td
  TARGET ClangAttrVisitor)

latino_tablegen(StmtNodes.inc -gen-clang-stmt-nodes
  SOURCE ../Basic/StmtNodes.td
  TARGET ClangStmtNodes)

latino_tablegen(DeclNodes.inc -gen-clang-decl-nodes
  SOURCE ../Basic/DeclNodes.td
  TARGET ClangDeclNodes)

latino_tablegen(TypeNodes.inc -gen-clang-type-nodes
  SOURCE ../Basic/TypeNodes.td
  TARGET ClangTypeNodes)

latino_tablegen(AbstractBasicReader.inc -gen-clang-basic-reader
  SOURCE PropertiesBase.td
  TARGET ClangAbstractBasicReader)

latino_tablegen(AbstractBasicWriter.inc -gen-clang-basic-writer
  SOURCE PropertiesBase.td
  TARGET ClangAbstractBasicWriter)

latino_tablegen(AbstractTypeReader.inc -gen-clang-type-reader
  SOURCE TypeProperties.td
  TARGET ClangAbstractTypeReader)

latino_tablegen(AbstractTypeWriter.inc -gen-clang-type-writer
  SOURCE TypeProperties.td
  TARGET ClangAbstractTypeWriter)

latino_tablegen(CommentNodes.inc -gen-clang-comment-nodes
  SOURCE ../Basic/CommentNodes.td
  TARGET ClangCommentNodes)

latino_tablegen(CommentHTMLTags.inc -gen-clang-comment-html-tags
  SOURCE CommentHTMLTags.td
  TARGET ClangCommentHTMLTags)

latino_tablegen(CommentHTMLTagsProperties.inc -gen-clang-comment-html-tags-properties
  SOURCE CommentHTMLTags.td
  TARGET ClangCommentHTMLTagsProperties)

latino_tablegen(CommentHTMLNamedCharacterReferences.inc -gen-clang-comment-html-named-character-references
  SOURCE CommentHTMLNamedCharacterReferences.td
  TARGET ClangCommentHTMLNamedCharacterReferences)

latino_tablegen(CommentCommandInfo.inc -gen-clang-comment-command-info
  SOURCE CommentCommands.td
  TARGET ClangCommentCommandInfo)

latino_tablegen(CommentCommandList.inc -gen-clang-comment-command-list
  SOURCE CommentCommands.td
  TARGET ClangCommentCommandList)

latino_tablegen(StmtDataCollectors.inc -gen-clang-data-collectors
  SOURCE StmtDataCollectors.td
  TARGET StmtDataCollectors)
