// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_PARSER_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_PARSER_H

#include "Bibble/lexer/Token.h"

#include "Bibble/parser/ast/expression/BinaryExpression.h"
#include "Bibble/parser/ast/expression/CallExpression.h"
#include "Bibble/parser/ast/expression/Integerliteral.h"
#include "Bibble/parser/ast/expression/MemberAccess.h"
#include "Bibble/parser/ast/expression/NewExpression.h"
#include "Bibble/parser/ast/expression/StringLiteral.h"
#include "Bibble/parser/ast/expression/VariableExpression.h"

#include "Bibble/parser/ast/statement/CompoundStatement.h"
#include "Bibble/parser/ast/statement/IfStatement.h"
#include "Bibble/parser/ast/statement/ReturnStatement.h"
#include "Bibble/parser/ast/statement/VariableDeclaration.h"

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

        lexer::Token current() const;
        lexer::Token consume();
        lexer::Token peek(int offset) const;

        void expectToken(lexer::TokenType tokenType);
        void expectAnyToken(lexer::TokenType first, auto... rest);

        int getBinaryOperatorPrecedence(lexer::TokenType tokenType);
        int getPrefixUnaryOperatorPrecedence(lexer::TokenType tokenType);
        int getPostfixUnaryOperatorPrecedence(lexer::TokenType tokenType);

        Type* parseType(bool failable = false, bool failableArray = false);

        ASTNodePtr parseGlobal();
        ASTNodePtr parseExpression(int precedence = 1);
        ASTNodePtr parsePrimary();

        FunctionPtr parseFunction(std::vector<lexer::Token> modifierTokens);
        ClassDeclarationPtr parseClass(std::vector<lexer::Token> modifierTokens);
        void parseClassMember(std::string_view className, std::vector<ClassField>& fields,
                              std::vector<ClassMethod>& constructors, std::vector<ClassMethod>& methods,
                              std::vector<lexer::Token> modifierTokens);

        void parseImport();

        ASTNodePtr parseParenExpression();
        IntegerLiteralPtr parseIntegerLiteral();
        IntegerLiteralPtr parseCharacterLiteral();
        StringLiteralPtr parseStringLiteral();
        MemberAccessPtr parseMemberAccess(ASTNodePtr classNode);
        VariableExpressionPtr parseVariableExpression();
        CallExpressionPtr parseCallExpression(ASTNodePtr callee);
        ASTNodePtr parseNewExpression();

        CompoundStatementPtr parseCompoundStatement();
        IfStatementPtr parseIfStatement();
        ReturnStatementPtr parseReturnStatement();
        VariableDeclarationPtr parseVariableDeclaration(Type* type); // type is parsed to see if it's a variable lmao
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
    MethodModifier GetMethodModifier(const lexer::Token& token, diagnostic::Diagnostics& diag);
    FunctionModifier GetFunctionModifier(const lexer::Token& token, diagnostic::Diagnostics& diag);
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_PARSER_PARSER_H
