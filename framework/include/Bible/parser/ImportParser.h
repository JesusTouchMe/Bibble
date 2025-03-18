// Copyright 2025 JesusTouchMe

#ifndef BIBLE_FRAMEWORK_INCLUDE_BIBLE_PARSER_IMPORTPARSER_H
#define BIBLE_FRAMEWORK_INCLUDE_BIBLE_PARSER_IMPORTPARSER_H

#include "Bible/lexer/Token.h"

#include "Bible/parser/ast/Node.h"

#include "Bible/symbol/Import.h"
#include "Bible/symbol/Scope.h"

#include <format>
#include <iostream>
#include <vector>

namespace parser {
    class ImportParser {
    public:
        ImportParser(std::vector<lexer::Token>& tokens, diagnostic::Diagnostics& diag, symbol::ImportManager& importManager);

        std::vector<ASTNodePtr> parse();

    private:
        std::vector<lexer::Token>& mTokens;
        int mPosition;

        symbol::ImportManager& mImportManager;

        symbol::Scope* mScope;

        diagnostic::Diagnostics& mDiag;

        lexer::Token current() const;
        lexer::Token consume();
        lexer::Token peek(int offset) const;

        void expectToken(lexer::TokenType type);
        void expectAnyToken(lexer::TokenType first, auto... rest);

        Type* parseType();


    };

    void ImportParser::expectAnyToken(lexer::TokenType first, auto ...rest) {
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

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_PARSER_IMPORTPARSER_H
