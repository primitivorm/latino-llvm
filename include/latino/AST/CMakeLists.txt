message(STATUS "latino/AST/CMakeLists.txt")

latino_tablegen(StmtNodes.inc -gen-clang-stmt-nodes
  SOURCE ../Basic/StmtNodes.td
  TARGET LatinoStmtNodes)

# latino_tablegen(DeclNodes.inc -gen-clang-decl-nodes
#     SOURCE ../Basic/DeclNodes.td 
#     TARGET LatinoDeclNodes)

latino_tablegen(TypeNodes.inc -gen-clang-type-nodes
  SOURCE ../Basic/TypeNodes.td
  TARGET LatinoTypeNodes)