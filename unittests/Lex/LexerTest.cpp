// #include "clang/Lex/Lexer.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
// #include "clang/Basic/TokenKinds.h"
// #include "clang/Lex/MacroArgs.h"
// #include "clang/Lex/MacroInfo.h"
#include "clang/Lex/ModuleLoader.h"
// #include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessorOptions.h"

#include "latino/Basic/LangOptions.h"
#include "latino/Lex/HeaderSearch.h"
#include "latino/Lex/HeaderSearchOptions.h"
#include "latino/Lex/Lexer.h"
#include "latino/Lex/Preprocessor.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <vector>

namespace {

using namespace latino;
using testing::ElementsAre;

class LexerTest : public ::testing::Test {
protected:
  LexerTest()
      : FileMgr(FileMgrOpts), DiagID(new clang::DiagnosticIDs()),
        Diags(DiagID, new clang::DiagnosticOptions,
              new clang::IgnoringDiagConsumer()),
        SourceMgr(Diags, FileMgr), TargetOpts(new clang::TargetOptions) {
    TargetOpts->Triple = "x86_64-apple-darwin11.1.0";
    Target = clang::TargetInfo::CreateTargetInfo(Diags, TargetOpts);
  }

  std::unique_ptr<Preprocessor>
  CreatePP(llvm::StringRef Source, clang::TrivialModuleLoader &ModLoader) {
    std::unique_ptr<llvm::MemoryBuffer> Buf =
        llvm::MemoryBuffer::getMemBuffer(Source);
    SourceMgr.setMainFileID(SourceMgr.createFileID(std::move(Buf)));

    HeaderSearch HeaderInfo(std::make_shared<HeaderSearchOptions>(), SourceMgr,
                            /*Diags,*/ LangOpts, Target.get());

    std::unique_ptr<Preprocessor> PP = std::make_unique<Preprocessor>(
        std::make_shared<clang::PreprocessorOptions>(), Diags, LangOpts,
        SourceMgr, HeaderInfo, ModLoader, /*IILookup*/ nullptr,
        /*OwnsHeaderSearch*/ false);

    PP->Initialize(*Target);
    PP->EnterMainSourceFile();

    return PP;
  }

  std::vector<Token> Lex(llvm::StringRef Source) {
    clang::TrivialModuleLoader ModLoader;
    auto PP = CreatePP(Source, ModLoader);

    std::vector<Token> toks;
    while (1) {
      Token tok;
      PP->Lex(tok);
      if (tok.is(tok::eof))
        break;
      toks.push_back(tok);
    }

    return toks;
  }

  std::vector<Token> CheckLex(llvm::StringRef Source,
                              llvm::ArrayRef<tok::TokenKind> ExpectedTokens) {
    auto toks = Lex(Source);
    EXPECT_EQ(ExpectedTokens.size(), toks.size());
    for (unsigned i = 0, e = ExpectedTokens.size(); i != e; ++i) {
      EXPECT_EQ(ExpectedTokens[i], toks[i].getKind());
    }

    return toks;
  }

  std::string getSourceText(Token Begin, Token End) {
    bool Invalid = false;
    llvm::StringRef Str = Lexer::getSourceText(
        clang::CharSourceRange::getTokenRange(
            clang::SourceRange(Begin.getLocation(), End.getLocation())),
        SourceMgr, LangOpts, &Invalid);
    if (Invalid)
      return "<INVALID>";
    return std::string(Str);
  }

  clang::FileSystemOptions FileMgrOpts;
  clang::FileManager FileMgr;
  clang::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID;
  clang::DiagnosticsEngine Diags;
  clang::SourceManager SourceMgr;
  LangOptions LangOpts;
  std::shared_ptr<clang::TargetOptions> TargetOpts;
  llvm::IntrusiveRefCntPtr<clang::TargetInfo> Target;
};

TEST_F(LexerTest, LexEndOfFile) {
  std::vector<tok::TokenKind> ExpectedTokens = {};
  std::vector<Token> toks = CheckLex("", ExpectedTokens);
}

TEST_F(LexerTest, SkipLineComment) {
  std::vector<tok::TokenKind> ExpectedTokens = {};
  llvm::StringRef code =
      "// Este es un comentario de una linea con final de linea.\n";
  std::vector<Token> toks = CheckLex(code, ExpectedTokens);
}

TEST_F(LexerTest, SkipLineCommentWithEOF) {
  std::vector<tok::TokenKind> ExpectedTokens = {};
  llvm::StringRef code =
      "// Este es un comentario de una linea con fin de archivo.";
  std::vector<Token> toks = CheckLex(code, ExpectedTokens);
}

TEST_F(LexerTest, SkipLineCommentWithCode) {
  std::vector<tok::TokenKind> ExpectedTokens;
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::colon);
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::equal);
  ExpectedTokens.push_back(tok::numeric_constant);
  llvm::StringRef code = "// Este es un comentario de una linea.\n "
                         "const PI: Entero = 3.1416";
  std::vector<Token> toks = CheckLex(code, ExpectedTokens);
}

TEST_F(LexerTest, SkipLineCommentHash) {
  std::vector<tok::TokenKind> ExpectedTokens = {};
  llvm::StringRef code = "# Este es un comentario de una linea.\n";
  std::vector<Token> toks = CheckLex(code, ExpectedTokens);
}

TEST_F(LexerTest, SkipBlockComment) {
  std::vector<tok::TokenKind> ExpectedTokens = {};
  llvm::StringRef code = "/* Este es un comentario\n"
                         "de dos linea con fin de linea. */\r";
  std::vector<Token> toks = CheckLex(code, ExpectedTokens);
}

TEST_F(LexerTest, SkipBlockCommentWithEOF) {
  std::vector<tok::TokenKind> ExpectedTokens = {};
  llvm::StringRef code = "/* Este es un comentario\n"
                         "de dos linea. */";
  std::vector<Token> toks = CheckLex(code, ExpectedTokens);
}

TEST_F(LexerTest, SkipBlockCommentWithBlockComment) {
  std::vector<tok::TokenKind> ExpectedTokens = {};
  llvm::StringRef code = "/* Este es un comentario\n"
                         "// const PI: Entero = 3.1416"
                         "con codigo comentado en medio. */\n";
  std::vector<Token> toks = CheckLex(code, ExpectedTokens);
}

TEST_F(LexerTest, SkipBlockCommentWithCode) {
  std::vector<tok::TokenKind> ExpectedTokens;
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::colon);
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::equal);
  ExpectedTokens.push_back(tok::numeric_constant);
  llvm::StringRef code = "/* Este es un comentario\n"
                         "de dos linea. */\n"
                         "const PI: Entero = 3.1416";
  std::vector<Token> toks = CheckLex(code, ExpectedTokens);
}

TEST_F(LexerTest, IsNewLineEscapedValid) {
  auto hasNewLineEscaped = [](const char *S) {
    return latino::Lexer::isNewLineEscaped(S, S + strlen(S) - 1);
  };

  EXPECT_TRUE(hasNewLineEscaped("\\\r"));
  EXPECT_TRUE(hasNewLineEscaped("\\\n"));
  EXPECT_TRUE(hasNewLineEscaped("\\\r\n"));
  EXPECT_TRUE(hasNewLineEscaped("\\\n\r"));
  EXPECT_TRUE(hasNewLineEscaped("\\ \t\v\f\r"));
  EXPECT_TRUE(hasNewLineEscaped("\\ \t\v\f\r\n"));

  EXPECT_FALSE(hasNewLineEscaped("\\\r\r"));
  EXPECT_FALSE(hasNewLineEscaped("\\\r\r\n"));
  EXPECT_FALSE(hasNewLineEscaped("\\\n\n"));
  EXPECT_FALSE(hasNewLineEscaped("\r"));
  EXPECT_FALSE(hasNewLineEscaped("\n"));
  EXPECT_FALSE(hasNewLineEscaped("\r\n"));
  EXPECT_FALSE(hasNewLineEscaped("\n\r"));
  EXPECT_FALSE(hasNewLineEscaped("\r\r"));
  EXPECT_FALSE(hasNewLineEscaped("\n\n"));
}

TEST_F(LexerTest, LexNumericConstant) {
  std::vector<tok::TokenKind> ExpectedTokens;
  ExpectedTokens.push_back(tok::numeric_constant);
  ExpectedTokens.push_back(tok::numeric_constant);
  ExpectedTokens.push_back(tok::numeric_constant);
  ExpectedTokens.push_back(tok::numeric_constant);
  ExpectedTokens.push_back(tok::numeric_constant);
  llvm::StringRef code = "10\n"
                         "0x1C\n"
                         "0o77\n"
                         "1.0\n"
                         "6.022e+23\n";
  std::vector<Token> toks = CheckLex(code, ExpectedTokens);
}

TEST_F(LexerTest, LexIdentifier) {
  std::vector<tok::TokenKind> ExpectedTokens;
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::raw_identifier);
  llvm::StringRef code = "camelCase\n"
                         "snake_case\n"
                         "PascalCase\n"
                         "SCREAMING_SNAKE_CASE\n"
                         "letrasNumeros1234\n"
                         "_guion_bajo_al_inicio\n"
                         "fUcKtHeCaSe\n"
                         "Cobra_Case\n";
  std::vector<Token> toks = CheckLex(code, ExpectedTokens);
}

TEST_F(LexerTest, FindNextToken) {
  Lex("var abcd: Entero = 0\n");
  std::vector<std::string> GeneratedByNextToken;
  SourceLocation Loc =
      SourceMgr.getLocForStartOfFile(SourceMgr.getMainFileID());
  while (true) {
    auto T = latino::Lexer::findNextToken(Loc, SourceMgr, LangOpts);
    ASSERT_TRUE(T.hasValue());
    if (T->is(tok::eof))
      break;
    std::string str = getSourceText(*T, *T);
    // std::cout << str << std::endl;
    GeneratedByNextToken.push_back(str);
    Loc = T->getLocation();
  }
  EXPECT_THAT(GeneratedByNextToken,
              ElementsAre("abcd", ":", "Entero", "=", "0"));
}

TEST_F(LexerTest, LexStringLiteral) {
  std::vector<tok::TokenKind> ExpectedTokens;
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::colon);
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::equal);
  ExpectedTokens.push_back(tok::string_literal);
  llvm::StringRef code = "const str: Cadena = \"Esto es una cadena\"\n";
  std::vector<Token> toks = CheckLex(code, ExpectedTokens);
}

TEST_F(LexerTest, LexStringLiteralWithTabs) {
  std::vector<tok::TokenKind> ExpectedTokens;
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::colon);
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::equal);
  ExpectedTokens.push_back(tok::string_literal);
  llvm::StringRef code = "const str: Cadena = \"Esto\t es\t una\t cadena\t "
                         "con\t tabs.\"\n";
  std::vector<Token> toks = CheckLex(code, ExpectedTokens);
}

TEST_F(LexerTest, LexStringLiteralWithTabsScaped) {
  std::vector<tok::TokenKind> ExpectedTokens;
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::colon);
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::equal);
  ExpectedTokens.push_back(tok::string_literal);
  llvm::StringRef code = "const str: Cadena = \"Esto\\t es\\t una\\t"
                         "cadena\\t con \\t tabs\\t escapados.\\t\"\n";
  std::vector<Token> toks = CheckLex(code, ExpectedTokens);
}

TEST_F(LexerTest, LexStringLiteralWithEOLScaped) {
  std::vector<tok::TokenKind> ExpectedTokens;
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::colon);
  ExpectedTokens.push_back(tok::raw_identifier);
  ExpectedTokens.push_back(tok::equal);
  ExpectedTokens.push_back(tok::string_literal);
  llvm::StringRef code =
      "const str: Cadena = \"Esto\\n es\\n una\\n cadena\\n\"\n";
  std::vector<Token> toks = CheckLex(code, ExpectedTokens);
}

} // namespace
