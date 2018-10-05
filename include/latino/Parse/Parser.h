//===----------------------------------------------------------------------===//
//
//  This file defines the Parser interface.
//
//===----------------------------------------------------------------------===//

#ifndef LATINO_PARSE_PARSER_H
#define LATINO_PARSE_PARSER_H

#include "latino/Basic/OperatorPrecedence.h"
#include "latino/Basic/Specifiers.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/SaveAndRestore.h"
#include <memory>
#include <stack>

namespace latino {
/// Parser - This implements a parser for the C family of languages.  After
/// parsing units of the grammar, productions are invoked to handle whatever has
/// been read.
///
class Parser {

  Lexer &Lex;

  /// Tok - The current token we are peeking ahead.  All parsing methods assume
  /// that this is valid.
  Token Tok;

  // PrevTokLocation - The location of the token we previously
  // consumed. This token is used for diagnostics where we expected to
  // see a token following another token (e.g., the ';' at the end of
  // a statement).
  SourceLocation PrevTokLocation;

  unsigned short ParenCount = 0, BracketCount = 0, BraceCount = 0;

  /// Actions - These are the callbacks we invoke as we parse various constructs
  /// in the file.
  Sema &Actions;

  DiagnosticsEngine &Diags;

public:
  Parser(Lexer &Lex, Sema &Actions, bool SkipFunctionBodies);
  ~Parser() override;

  const LangOptions &getLangOpts() const { return Lex.getLangOpts(); }
  const TargetInfo &getTargetInfo() const { return Lex.getTargetInfo(); }
  Sema &getActions() const { return Actions; };

  const Token &getCurToken() const { return Tok; }
  Scope *getCurScope() const { Actions.getCurScope(); }

  Decl *getObjDeclContext() const { return Actions.getObjDeclContext(); }

}; /* class Parser */
} /* namespace latino */

#endif /* LATINO_PARSE_PARSER_H */
