// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_PARSER_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_PARSER_H

#include "Bibble/lexer/Token.h"

#include "Bibble/parser/ast/expression/BinaryExpression.h"
#include "Bibble/parser/ast/expression/CallExpression.h"
#include "Bibble/parser/ast/expression/Integerliteral.h"
#include "Bibble/parser/ast/expression/VariableExpression.h"

#include "Bibble/parser/ast/global/ClassDeclaration.h"
#include "Bibble/parser/ast/global/Function.h"

#include "Bibble/symbol/Import.h"

#include <format>
#include <functional>

namespace parser {
    class Parser {
    public:
        Parser(std::vector<lexer::Token>& tokens, diagnostic::Diagnostics& diag, symbol::ImportManager& importManager, symbol::Scope* globalScope);

        std::vector<ASTNodePtr> parse();

    private:
        std::vector<lexer::Token>& mTokens;
        std::size_t mPosition;

        diagnostic::Diagnostics& mDiag;

        symbol::Scope* mScope;

        symbol::ImportManager& mImportManager;

        std::function<void(ASTNodePtr&)> mInsertNode;

        lexer::Token current() const;
        lexer::Token consume();
        lexer::Token peek(int offset) const;

        void expectToken(lexer::TokenType tokenType);
        void expectAnyToken(lexer::TokenType first, auto... rest);

        int getBinaryOperatorPrecedence(lexer::TokenType tokenType);
        int getPrefixUnaryOperatorPrecedence(lexer::TokenType tokenType);
        int getPostfixUnaryOperatorPrecedence(lexer::TokenType tokenType);

        Type* parseType();

        ASTNodePtr parseGlobal();
        ASTNodePtr parseExpression(int precedence = 1);
        ASTNodePtr parsePrimary();

        FunctionPtr parseFunction(std::vector<lexer::Token> modifierTokens);
        ClassDeclarationPtr parseClass(std::vector<lexer::Token> modifierTokens);
        void parseClassMember(std::vector<ClassField>& fields, std::vector<ClassMethod>& methods, std::vector<lexer::Token> modifierTokens);

        void parseImport();

        IntegerLiteralPtr parseIntegerLiteral();
        VariableExpressionPtr parseVariableExpression();
        CallExpressionPtr parseCallExpression(ASTNodePtr callee);
    };

    void Parser::expectAnyToken(lexer::TokenType first, auto... rest) {
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

    bool IsModifierToken(const lexer::Token& token);

    ClassModifier GetClassModifier(const lexer::Token& token, diagnostic::Diagnostics& diag);
    FieldModifier GetFieldModifier(const lexer::Token& token, diagnostic::Diagnostics& diag);
    FunctionModifier GetFunctionModifier(const lexer::Token& token, diagnostic::Diagnostics& diag);
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_PARSER_H
