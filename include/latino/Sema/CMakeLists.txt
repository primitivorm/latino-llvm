latino_tablegen(AttrTemplateInstantiate.inc -gen-clang-attr-template-instantiate
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  SOURCE ../Basic/Attr.td
  TARGET LatinoAttrTemplateInstantiate)
  
latino_tablegen(AttrParsedAttrList.inc -gen-clang-attr-parsed-attr-list
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  SOURCE ../Basic/Attr.td
  TARGET LatinoAttrParsedAttrList)

latino_tablegen(AttrParsedAttrKinds.inc -gen-clang-attr-parsed-attr-kinds
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  SOURCE ../Basic/Attr.td
  TARGET LatinoAttrParsedAttrKinds)

latino_tablegen(AttrSpellingListIndex.inc -gen-clang-attr-spelling-index
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  SOURCE ../Basic/Attr.td
  TARGET LatinoAttrSpellingListIndex)

latino_tablegen(AttrParsedAttrImpl.inc -gen-clang-attr-parsed-attr-impl
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  SOURCE ../Basic/Attr.td
  TARGET LatinoAttrParsedAttrImpl)
