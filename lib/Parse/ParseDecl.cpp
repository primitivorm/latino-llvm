#include "latino/Parse/Parser.h"

#include "clang/Basic/CharInfo.h"

#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringSwitch.h"

using namespace latino;

/// isDeclarationSpecifier() - Return true if the current token is part of a
/// declaration specifier.
///
/// \param DisambiguatingWithExpression True to indicate that the purpose of
/// this check is to disambiguate between an expression and a declaration.
bool Parser::isDeclarationSpecifier(bool DisambiguatingWithExpression) {
  switch (Tok.getKind()) {
  default:
    return false;

    // case tok::kw_pipe:
    //     return (getLangOpts().OpenCL && getLangOpts().OpenCLVersion >= 200)
    //     ||
    //        getLangOpts().OpenCLCPlusPlus;

    // case tok::identifier: // foo::bar
    // Unfortunate hack to support "Class.factoryMethod" notation.
    // if (getLangOpts().ObjC && NextToken().is(tok::period))
    //   return false;
    // if (TryAltiVecVectorToken())
    //   return true;
    // LLVM_FALLTHROUGH;

    // case tok::kw_externo:
    //   return true;
  }
}