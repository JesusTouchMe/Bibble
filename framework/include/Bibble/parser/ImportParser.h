// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_IMPORTPARSER_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_IMPORTPARSER_H

#include "Bibble/lexer/Token.h"

#include "Bibble/parser/ast/global/ClassDeclaration.h"
#include "Bibble/parser/ast/global/Function.h"

#include "Bibble/symbol/Import.h"
#include "Bibble/symbol/Scope.h"

#include <format>
#include <iostream>
#include <vector>

namespace parser {
    // Currently unused, but exists for the future when I wanna make all symbols be forward declared
    class ImportParser {
    public:
        ImportParser(std::vector<lexer::Token>& tokens, diagnostic::Diagnostics& diag, symbol::ImportManager& importManager, symbol::Scope* globalScope);

        std::vector<ASTNodePtr> parse();

    private:
        std::vector<lexer::Token>& mTokens;
        std::size_t mPosition;

        symbol::ImportManager& mImportManager;

        symbol::Scope* mScope;

        diagnostic::Diagnostics& mDiag;

        lexer::Token current() const;
        lexer::Token consume();
        lexer::Token peek(int offset) const;

        void expectToken(lexer::TokenType type);
        void expectAnyToken(lexer::TokenType first, auto... rest);

        Type* parseType();

        ASTNodePtr parseGlobal();

        void parseImport();

        FunctionPtr parseFunction(std::vector<lexer::Token> modifierTokens);
        ClassDeclarationPtr parseClass(std::vector<lexer::Token> modifierTokens);
        void parseClassMember(std::string_view className, std::vector<ClassField>& fields,
                              std::vector<ClassMethod>& constructors, std::vector<ClassMethod>& methods,
                              const std::vector<lexer::Token>& modifierTokens);
    };

    void ImportParser::expectAnyToken(lexer::TokenType first, auto... rest) {
        if constexpr (sizeof...(rest) == 0) {
            if (current().getTokenType() == first)
                return;
        } else {
            if (((current().getTokenType() == first) || ... || (current().getTokenType() == rest)))
                return;
        }

        std::string tokensString;
        auto appendToken = [&](lexer::TokenType type) {
            lexer::Token temp("", type, lexer::SourceLocation(), lexer::SourceLocation());
            tokensString += std::format("'{}{}{}', ", fmt::bold, temp.getName(), fmt::defaults);
        };

        appendToken(first);
        (appendToken(rest), ...);

        tokensString = tokensString.substr(0, tokensString.size() - 2);

        mDiag.compilerError(current().getStartLocation(), current().getEndLocation(),
                            std::format("expected either {} before '{}{}{}'",
                                        tokensString, fmt::bold, current().getName(), fmt::defaults));
    }
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_IMPORTPARSER_H
