// Copyright 2025 JesusTouchMe

#include "Bibble/parser/Parser.h"

#include "Bibble/parser/ast/expression/UnaryExpression.h"

#include <cinttypes>
#include <format>
#include <utility>

namespace parser {
    Parser::Parser(std::vector<lexer::Token>& tokens, diagnostic::Diagnostics& diag,
                   symbol::ImportManager& importManager, symbol::Scope* globalScope)
                   : mTokens(tokens)
                   , mPosition(0)
                   , mDiag(diag)
                   , mImportManager(importManager)
                   , mScope(globalScope) {}

    std::vector<ASTNodePtr> Parser::parse() {
        std::vector<ASTNodePtr> ast;

        mInsertNode = [&ast](ASTNodePtr& node) {
            if (node) ast.push_back(std::move(node));
        };

        while (mPosition < mTokens.size()) {
            auto global = parseGlobal();
            if (global) {
                ast.push_back(std::move(global));
            }
        }

        return ast;
    }

    lexer::Token Parser::current() const {
        return mTokens[mPosition];
    }

    lexer::Token Parser::consume() {
        return mTokens[mPosition++];
    }

    lexer::Token Parser::peek(int offset) const {
        return mTokens[mPosition + offset];
    }

    void Parser::expectToken(lexer::TokenType tokenType) {
        if (current().getTokenType() != tokenType)
        {
            lexer::Token temp("", tokenType, lexer::SourceLocation(), lexer::SourceLocation());
            mDiag.compilerError(current().getStartLocation(),
                                current().getEndLocation(),
                                std::format("expected '{}{}{}', found '{}{}{}'",
                                            fmt::bold, temp.getName(), fmt::defaults,
                                            fmt::bold, current().getText(), fmt::defaults));
            std::exit(1);
        }
    }

    int Parser::getBinaryOperatorPrecedence(lexer::TokenType tokenType) {
        switch (tokenType) {
            case lexer::TokenType::LeftParen:
            case lexer::TokenType::Dot:
                return 90;

            case lexer::TokenType::Star:
            case lexer::TokenType::Slash:
                return 75;
            case lexer::TokenType::Plus:
            case lexer::TokenType::Minus:
                return 70;

            case lexer::TokenType::Equal:
                return 20;

            default:
                return 0;
        }
    }

    int Parser::getPrefixUnaryOperatorPrecedence(lexer::TokenType tokenType) {
        switch(tokenType) {
            case lexer::TokenType::Minus:
                return 85;

            default:
                return 0;
        }
    }

    int Parser::getPostfixUnaryOperatorPrecedence(lexer::TokenType tokenType) {
        return 0;
    }

    Type* Parser::parseType() {
        Type* type = nullptr;

        if (current().getTokenType() == lexer::TokenType::Type) {
            type = Type::Get(consume().getText());
        } else {
            //TODO: implement using full module names instead of an imported module name
            expectToken(lexer::TokenType::Identifier);

            lexer::SourceLocation start = current().getStartLocation();
            std::string_view moduleName = mScope->findModuleName(consume().getText());

            expectToken(lexer::TokenType::Dot);
            consume();

            expectToken(lexer::TokenType::Identifier);

            lexer::SourceLocation end = current().getEndLocation();
            std::string_view className = consume().getText();

            type = ClassType::Create(moduleName, className);
        }

        return type;
    }

    ASTNodePtr Parser::parseGlobal() {
        std::vector<lexer::Token> modifierTokens;

        while (IsModifierToken(current())) {
            modifierTokens.push_back(consume());
        }

        switch (current().getTokenType()) {
            case lexer::TokenType::ImportKeyword:
                if (!modifierTokens.empty()) {
                    mDiag.compilerError(modifierTokens.front().getStartLocation(),
                                        current().getEndLocation(),
                                        std::format("using modifiers for '{}import{}'",
                                                    fmt::bold, fmt::defaults));
                    std::exit(1);
                }

                parseImport();
                return nullptr;

            case lexer::TokenType::Type:
            case lexer::TokenType::Identifier:
                return parseFunction(std::move(modifierTokens));

            case lexer::TokenType::EndOfFile:
                consume();
                return nullptr;

            default:
                mDiag.compilerError(current().getStartLocation(),
                                    current().getEndLocation(),
                                    std::format("expected global expression, found '{}{}{}'",
                                                fmt::bold, current().getText(), fmt::defaults));
                std::exit(1);
        }
    }

    ASTNodePtr Parser::parseExpression(int precedence) {
        ASTNodePtr left;

        int prefixOperatorPrecedence = getPrefixUnaryOperatorPrecedence(current().getTokenType());

        if (prefixOperatorPrecedence >= precedence) {
            lexer::Token operatorToken = consume();
            left = std::make_unique<UnaryExpression>(mScope, parseExpression(prefixOperatorPrecedence), operatorToken.getTokenType(), false, std::move(operatorToken));
        } else {
            left = parsePrimary();
        }

        while (true) {
            int postfixOperatorPrecedence = getPostfixUnaryOperatorPrecedence(current().getTokenType());
            if (postfixOperatorPrecedence < precedence) {
                break;
            }

            lexer::Token operatorToken = consume();

            left = std::make_unique<UnaryExpression>(mScope, std::move(left), operatorToken.getTokenType(), true, std::move(operatorToken));
        }

        while (true) {
            int binaryOperatorPrecedence = getBinaryOperatorPrecedence(current().getTokenType());
            if (binaryOperatorPrecedence < precedence) {
                break;
            }

            lexer::Token operatorToken = consume();

            if (operatorToken.getTokenType() == lexer::TokenType::LeftParen) {
                left = parseCallExpression(std::move(left));
            } else {
                ASTNodePtr right = parseExpression(binaryOperatorPrecedence);
                left = std::make_unique<BinaryExpression>(mScope, std::move(left), operatorToken.getTokenType(), std::move(right), std::move(operatorToken));
            }
        }

        return left;
    }

    ASTNodePtr Parser::parsePrimary() {
        switch (current().getTokenType()) {
            case lexer::TokenType::IntegerLiteral:
                return parseIntegerLiteral();

            case lexer::TokenType::Identifier:
                return parseVariableExpression();

            default:
                mDiag.compilerError(current().getStartLocation(),
                                    current().getEndLocation(),
                                    std::format("expected primary expression, found '{}{}{}'",
                                                fmt::bold, current().getText(), fmt::defaults));
                std::exit(1);
        }
    }

    FunctionPtr Parser::parseFunction(std::vector<lexer::Token> modifierTokens) {
        bool native = false;

        std::vector<FunctionModifier> modifiers;
        for (auto& token : modifierTokens) {
            auto modifier = GetFunctionModifier(token, mDiag);
            if (std::find(modifiers.begin(), modifiers.end(), modifier) != modifiers.end()) {
                mDiag.compilerError(token.getStartLocation(),
                                    token.getEndLocation(),
                                    std::format("duplicate function modifier: '{}{}{}'",
                                                fmt::bold, token.getText(), fmt::defaults));
                std::exit(EXIT_FAILURE);
            }

            if (modifier == FunctionModifier::Native) native = true;
            modifiers.push_back(modifier);
        }

        auto token = current();
        Type* returnType = parseType();

        expectToken(lexer::TokenType::Identifier);
        auto nameToken = consume();
        std::string name = std::string(nameToken.getText());

        std::vector<FunctionArgument> arguments;
        std::vector<Type*> argumentTypes;

        expectToken(lexer::TokenType::LeftParen);
        consume();

        while (current().getTokenType() != lexer::TokenType::RightParen) {
            Type* type = parseType();

            expectToken(lexer::TokenType::Identifier);
            std::string argumentName = std::string(consume().getText());

            arguments.emplace_back(type, std::move(argumentName));
            argumentTypes.push_back(type);

            if (current().getTokenType() != lexer::TokenType::RightParen) {
                expectToken(lexer::TokenType::Comma);
                consume();
            }
        }
        consume();

        FunctionType* functionType = FunctionType::Create(returnType, std::move(argumentTypes));

        symbol::ScopePtr scope = std::make_unique<symbol::Scope>(mScope, "", false, returnType);
        mScope = scope.get();

        if (native) {
            expectToken(lexer::TokenType::Semicolon);
            consume();
            mScope = scope->parent;

            return std::make_unique<Function>(std::move(modifiers), std::move(name), functionType, std::move(arguments), std::vector<ASTNodePtr>(), std::move(scope), std::move(token));
        }

        expectToken(lexer::TokenType::LeftBrace);
        consume();

        std::vector<ASTNodePtr> body;
        while (current().getTokenType() != lexer::TokenType::RightBrace) {
            body.push_back(parseExpression());
            expectToken(lexer::TokenType::Semicolon);
            consume();
        }
        consume();

        mScope = scope->parent;

        return std::make_unique<Function>(std::move(modifiers), std::move(name), functionType, std::move(arguments), std::move(body), std::move(scope), std::move(token));
    }

    ClassDeclarationPtr Parser::parseClass(std::vector<lexer::Token> modifierTokens) {
        return nullptr;
    }

    void Parser::parseClassMember(std::vector<ClassField>& fields, std::vector<ClassMethod>& methods, std::vector<lexer::Token> modifierTokens) {

    }

    void Parser::parseImport() {
        consume();

        fs::path path;
        std::string moduleName;
        while (current().getTokenType() != lexer::TokenType::Semicolon) {
            expectToken(lexer::TokenType::Identifier);
            path /= current().getText();
            moduleName += consume().getText();

            if (current().getTokenType() != lexer::TokenType::Semicolon) {
                expectToken(lexer::TokenType::Dot);
                consume();

                moduleName += "/";
            }
        }
        consume();

        symbol::ScopePtr scope = std::make_unique<symbol::Scope>(nullptr, moduleName, true);

        auto nodes = mImportManager.importModule(path, mDiag, scope.get());

        mScope->children.push_back(scope.get());

        mImportManager.seizeScope(std::move(scope));

        std::string shortModuleName;
        auto pos = moduleName.find_last_of('/');

        if (pos == std::string::npos) {
            shortModuleName = moduleName;
        } else {
            shortModuleName = moduleName.substr(pos + 1);
        }

        mScope->getTopLevelScope()->importedModuleNames[std::move(shortModuleName)] = std::move(moduleName);
    }

    IntegerLiteralPtr Parser::parseIntegerLiteral() {
        auto token = consume();
        std::string text = std::string(token.getText());

        return std::make_unique<IntegerLiteral>(mScope, std::strtoimax(text.c_str(), nullptr, 0), std::move(token));
    }

    VariableExpressionPtr Parser::parseVariableExpression() {
        auto token = consume();
        std::vector<std::string> names;
        names.emplace_back(mScope->findModuleName(token.getText()));

        while (current().getTokenType() == lexer::TokenType::DoubleColon) {
            consume();
            expectToken(lexer::TokenType::Identifier);
            token = consume();
            names.emplace_back(mScope->findModuleName(token.getText()));
        }

        if (names.size() == 1) {
            return std::make_unique<VariableExpression>(mScope, std::move(names[0]), std::move(token));
        } else {
            return std::make_unique<VariableExpression>(mScope, std::move(names), std::move(token));
        }
    }

    CallExpressionPtr Parser::parseCallExpression(ASTNodePtr callee) {
        std::vector<ASTNodePtr> parameters;
        while (current().getTokenType() != lexer::TokenType::RightParen) {
            parameters.push_back(parseExpression());
            if (current().getTokenType() != lexer::TokenType::RightParen) {
                expectToken(lexer::TokenType::Comma);
                consume();
            }
        }
        consume();

        return std::make_unique<CallExpression>(mScope, std::move(callee), std::move(parameters));
    }

    bool IsModifierToken(const lexer::Token& token) {
        return token.getTokenType() == lexer::TokenType::NativeKeyword;
    }

    ClassModifier GetClassModifier(const lexer::Token& token, diagnostic::Diagnostics& diag) {
        diag.fatalError("todo");
    }

    FieldModifier GetFieldModifier(const lexer::Token& token, diagnostic::Diagnostics& diag) {
        diag.fatalError("todo");
    }

    FunctionModifier GetFunctionModifier(const lexer::Token& token, diagnostic::Diagnostics& diag) {
        switch (token.getTokenType()) {
            case lexer::TokenType::NativeKeyword:
                return FunctionModifier::Native;

            default:
                diag.compilerError(token.getStartLocation(),
                                   token.getEndLocation(),
                                   std::format("invalid function modifier: '{}{}{}'",
                                               fmt::bold, token.getText(), fmt::defaults));
        }
    }
}