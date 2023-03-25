set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  MC
  MCParser
  Support
  )

add_latino_library(latinoParse
  ParseAST.cpp
  ParseCXXInlineMethods.cpp
  ParseDecl.cpp
  ParseDeclCXX.cpp
  ParseExpr.cpp
  ParseExprCXX.cpp
  ParseInit.cpp
  ParseObjc.cpp
  ParseOpenMP.cpp
  ParsePragma.cpp
  ParseStmt.cpp
  ParseStmtAsm.cpp
  ParseTemplate.cpp
  ParseTentative.cpp
  Parser.cpp

  LINK_LIBS
  latinoAST
  latinoBasic
  latinoLex
  latinoSema

  DEPENDS
  omp_gen
  )
