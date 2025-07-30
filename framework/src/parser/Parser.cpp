// Copyright 2025 JesusTouchMe

#include "Bibble/parser/Parser.h"

#include "Bibble/parser/ast/expression/BinaryExpression.h"
#include "Bibble/parser/ast/expression/BooleanLiteral.h"
#include "Bibble/parser/ast/expression/CastExpression.h"
#include "Bibble/parser/ast/expression/NewArrayExpression.h"
#include "Bibble/parser/ast/expression/NullLiteral.h"
#include "Bibble/parser/ast/expression/UnaryExpression.h"

#include "Bibble/type/ArrayType.h"
#include "Bibble/type/ViewType.h"

#include <cinttypes>
#include <filesystem>
#include <format>
#include <utility>

namespace parser {
    Parser::Parser(std::vector<lexer::Token>& tokens, diagnostic::Diagnostics& diag,
                   symbol::Scope* globalScope, bool importer)
                   : mTokens(tokens)
                   , mPosition(0)
                   , mDiag(diag)
                   , mScope(globalScope)
                   , mImporter(importer) {}

    std::vector<ASTNodePtr> Parser::parse() {
        std::vector<ASTNodePtr> ast;

        ast.push_back(std::make_unique<ImportStatement>(mScope, std::vector<std::string>{"std", "Primitives"}, lexer::Token()));

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
            case lexer::TokenType::LeftBracket:
            case lexer::TokenType::Dot:
                return 90;

            case lexer::TokenType::Star:
            case lexer::TokenType::Slash:
                return 75;
            case lexer::TokenType::Plus:
            case lexer::TokenType::Minus:
                return 70;

            case lexer::TokenType::LessThan:
            case lexer::TokenType::GreaterThan:
            case lexer::TokenType::LessEqual:
            case lexer::TokenType::GreaterEqual:
                return 55;

            case lexer::TokenType::DoubleEqual:
            case lexer::TokenType::BangEqual:
                return 50;

            case lexer::TokenType::DoubleAmpersand:
                return 30;

            case lexer::TokenType::DoublePipe:
                return 25;

            case lexer::TokenType::Equal:
            case lexer::TokenType::PlusEqual:
            case lexer::TokenType::MinusEqual:
                return 20;

            default:
                return 0;
        }
    }

    int Parser::getPrefixUnaryOperatorPrecedence(lexer::TokenType tokenType) {
        switch (tokenType) {
            case lexer::TokenType::LeftParen:
            case lexer::TokenType::Minus:
                return 85;

            case lexer::TokenType::DoublePlus:
            case lexer::TokenType::DoubleMinus:
                return 80;


            default:
                return 0;
        }
    }

    int Parser::getPostfixUnaryOperatorPrecedence(lexer::TokenType tokenType) {
        switch (tokenType) {
            case lexer::TokenType::DoublePlus:
            case lexer::TokenType::DoubleMinus:
                return 90;

            default:
                return 0;
        }
    }

    Type* Parser::parseType(bool failable, bool failableArray) {
        auto ExpectToken = [this, failable](lexer::TokenType type) {
            if (failable) {
                return current().getTokenType() != type;
            } else {
                expectToken(type);
                return false;
            }
        };

        lexer::Token token;

        auto startPosition = mPosition;
        Type* type = nullptr;

        if (current().getTokenType() == lexer::TokenType::Type) {
            token = current();
            type = Type::Get(consume().getText());
        } else if (current().getTokenType() == lexer::TokenType::ViewKeyword) {
            token = consume();

            expectToken(lexer::TokenType::LessThan);
            consume();

            Type* baseType = parseType(failable, failableArray);

            expectToken(lexer::TokenType::GreaterThan);
            consume();

            if (baseType != nullptr) {
                if (!baseType->isReferenceType()) {
                    mDiag.compilerError(token.getStartLocation(),
                                        current().getEndLocation(),
                                        std::format("cannot create view of non-reference type '{}{}{}'",
                                                    fmt::bold, baseType->getName(), fmt::defaults));
                    std::exit(1);
                }

                type = ViewType::Create(baseType);
            }
        } else {
            //TODO: implement using full module names instead of an imported module name
            if (ExpectToken(lexer::TokenType::Identifier)) {
                mPosition = startPosition;
                return nullptr;
            }

            std::vector<std::string> names;
            names.emplace_back(mScope->findModuleName(consume().getText()));

            while (current().getTokenType() == lexer::TokenType::DoubleColon) {
                consume();
                if (ExpectToken(lexer::TokenType::Identifier)) {
                    mPosition = startPosition;
                    return nullptr;
                }

                token = consume();
                names.emplace_back(mScope->findModuleName(token.getText()));
            }

            token = peek(-1);

            if (names.size() == 1) type = ClassType::Find(mScope->findModuleScope()->name, names.back());
            else if (names.size() == 2) type = ClassType::Find(names[0], names[1]);
            else {
                auto symbol = mScope->findClass(names);
                if (symbol != nullptr) type = symbol->getType();
            }
        }

        if (type == nullptr) {
            if (failable) {
                mPosition = startPosition;
                return nullptr;
            } else {
                mDiag.compilerError(token.getStartLocation(),
                                    token.getEndLocation(),
                                    std::format("unknown type name '{}{}{}'", fmt::bold, token.getText(), fmt::defaults));
                std::exit(1);
            }
        }

        while (current().getTokenType() == lexer::TokenType::LeftBracket) {
            consume();
            if (failableArray) {
                if (current().getTokenType() != lexer::TokenType::RightBracket) {
                    mPosition--;
                    break;
                }
            } else if (ExpectToken(lexer::TokenType::RightBracket)) {
                mPosition = startPosition;
                return nullptr;
            }

            consume();

            type = ArrayType::Create(type);
        }

        return type;
    }

    ASTNodePtr Parser::parseGlobal() {
        std::vector<lexer::Token> modifierTokens;

        while (IsModifierToken(current())) {
            modifierTokens.push_back(consume());
        }

        if (current().getTokenType() == lexer::TokenType::VarKeyword) {
            consume();
            return parseGlobalVar(std::move(modifierTokens), nullptr);
        }
        if (Type* type = parseType(true)) {
            if (peek(1).getTokenType() == lexer::TokenType::LeftParen) {
                return parseFunction(std::move(modifierTokens), type);
            } else {
                return parseGlobalVar(std::move(modifierTokens), type);
            }
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

                return parseImport();

            case lexer::TokenType::ClassKeyword:
                return parseClass(std::move(modifierTokens));

            case lexer::TokenType::LeftBrace:
                if (!modifierTokens.empty()) {
                    mDiag.compilerError(modifierTokens.front().getStartLocation(),
                                        current().getEndLocation(),
                                        std::format("using modifiers for {}init block{}",
                                                    fmt::bold, fmt::defaults));
                    std::exit(1);
                }

            return parseInitBlock();

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
            if (operatorToken.getTokenType() == lexer::TokenType::LeftParen) {
                if (Type* type = parseType(true)) {
                    expectToken(lexer::TokenType::RightParen);
                    consume();
                    left = std::make_unique<CastExpression>(mScope, parseExpression(prefixOperatorPrecedence), type);
                } else {
                    left = parseParenExpression();
                }
            } else {
                left = std::make_unique<UnaryExpression>(mScope, parseExpression(prefixOperatorPrecedence), operatorToken.getTokenType(), false, std::move(operatorToken));
            }
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
            } else if (operatorToken.getTokenType() == lexer::TokenType::LeftBracket) {
                ASTNodePtr index = parseExpression();

                expectToken(lexer::TokenType::RightBracket);
                consume();

                left = std::make_unique<BinaryExpression>(mScope, std::move(left), BinaryExpression::Operator::Index, std::move(index), std::move(operatorToken));
            } else if (operatorToken.getTokenType() == lexer::TokenType::Dot) {
                left = parseMemberAccess(std::move(left));
            } else {
                ASTNodePtr right = parseExpression(binaryOperatorPrecedence);
                left = std::make_unique<BinaryExpression>(mScope, std::move(left), operatorToken.getTokenType(), std::move(right), std::move(operatorToken));
            }
        }

        return left;
    }

    ASTNodePtr Parser::parsePrimary() {
        if (current().getTokenType() == lexer::TokenType::VarKeyword) {
            consume();
            return parseVariableDeclaration(nullptr);
        }
        if (Type* type = parseType(true)) {
            return parseVariableDeclaration(type);
        }

        switch (current().getTokenType()) {
            case lexer::TokenType::TrueKeyword:
                return std::make_unique<BooleanLiteral>(mScope, true, consume());
            case lexer::TokenType::FalseKeyword:
                return std::make_unique<BooleanLiteral>(mScope, false, consume());
            case lexer::TokenType::NullKeyword:
                return std::make_unique<NullLiteral>(mScope, consume());

            case lexer::TokenType::IntegerLiteral:
                return parseIntegerLiteral();

            case lexer::TokenType::CharacterLiteral:
                return parseCharacterLiteral();

            case lexer::TokenType::StringLiteral:
                return parseStringLiteral();

            case lexer::TokenType::Identifier:
                return parseVariableExpression();

            case lexer::TokenType::NewKeyword:
                return parseNewExpression();

            case lexer::TokenType::LeftBrace:
                return parseCompoundStatement();

            case lexer::TokenType::IfKeyword:
                return parseIfStatement();

            case lexer::TokenType::WhileKeyword:
                return parseWhileStatement();

            case lexer::TokenType::ForKeyword:
                return parseForStatement();

            case lexer::TokenType::BreakKeyword:
                return parseBreakStatement();

            case lexer::TokenType::ContinueKeyword:
                return parseContinueStatement();

            case lexer::TokenType::ReturnKeyword:
                return parseReturnStatement();

            default:
                mDiag.compilerError(current().getStartLocation(),
                                    current().getEndLocation(),
                                    std::format("expected primary expression, found '{}{}{}'",
                                                fmt::bold, current().getText(), fmt::defaults));
                std::exit(1);
        }
    }

    FunctionPtr Parser::parseFunction(std::vector<lexer::Token> modifierTokens, Type* returnType) {
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

        if (std::find(modifiers.begin(), modifiers.end(), FunctionModifier::Public) == modifiers.end() &&
            std::find(modifiers.begin(), modifiers.end(), FunctionModifier::Private) == modifiers.end()) {
            modifiers.push_back(FunctionModifier::Public);
        }

        expectToken(lexer::TokenType::Identifier);
        auto token = consume();
        std::string name = std::string(token.getText());

        std::vector<FunctionArgument> arguments;
        std::vector<Type*> argumentTypes;

        bool callerLocation = false;

        expectToken(lexer::TokenType::LeftParen);
        consume();

        while (current().getTokenType() != lexer::TokenType::RightParen) {
            if (current().getTokenType() == lexer::TokenType::Identifier && current().getText() == "__compiler_caller_location") {
                consume();
                expectToken(lexer::TokenType::LeftParen);
                consume();

                expectToken(lexer::TokenType::Identifier);
                std::string file(consume().getText());

                expectToken(lexer::TokenType::Comma);
                consume();

                expectToken(lexer::TokenType::Identifier);
                std::string line(consume().getText());

                expectToken(lexer::TokenType::Comma);
                consume();

                expectToken(lexer::TokenType::Identifier);
                std::string column(consume().getText());

                expectToken(lexer::TokenType::RightParen);
                consume();

                expectToken(lexer::TokenType::RightParen);

                callerLocation = true;

                arguments.emplace_back(Type::Get("string"), std::move(file));
                argumentTypes.push_back(Type::Get("string"));

                arguments.emplace_back(Type::Get("int"), std::move(line));
                argumentTypes.push_back(Type::Get("int"));

                arguments.emplace_back(Type::Get("int"), std::move(column));
                argumentTypes.push_back(Type::Get("int"));
                break;
            }

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
        scope->currentVariableIndex = 0;
        mScope = scope.get();

        if (native) {
            expectToken(lexer::TokenType::Semicolon);
            consume();
            mScope = scope->parent;

            return std::make_unique<Function>(std::move(modifiers), std::move(name), functionType, std::move(arguments), callerLocation, std::vector<ASTNodePtr>(), std::move(scope), std::move(token));
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

        if (mImporter) {
            body.clear();
        }

        return std::make_unique<Function>(std::move(modifiers), std::move(name), functionType, std::move(arguments), callerLocation, std::move(body), std::move(scope), std::move(token));
    }

    ClassDeclarationPtr Parser::parseClass(std::vector<lexer::Token> modifierTokens) {
        std::vector<ClassModifier> modifiers;
        for (auto& token : modifierTokens) {
            auto modifier = GetClassModifier(token, mDiag);
            if (std::find(modifiers.begin(), modifiers.end(), modifier) != modifiers.end()) {
                mDiag.compilerError(token.getStartLocation(),
                                    token.getEndLocation(),
                                    std::format("duplicate class modifier: '{}{}{}'",
                                                fmt::bold, token.getText(), fmt::defaults));
                std::exit(1);
            }

            modifiers.push_back(modifier);
        }

        if (std::find(modifiers.begin(), modifiers.end(), ClassModifier::Public) == modifiers.end() &&
            std::find(modifiers.begin(), modifiers.end(), ClassModifier::Private) == modifiers.end()) {
            modifiers.push_back(ClassModifier::Public);
        }

        consume(); // class

        auto token = current();
        std::string name = std::string(consume().getText());

        ClassType* baseType;

        if (current().getTokenType() == lexer::TokenType::ExtendsKeyword) {
            auto extendsStart = current().getStartLocation();

            consume();

            Type* type = parseType();

            if (!type->isClassType()) {
                mDiag.compilerError(extendsStart,
                                    peek(-1).getEndLocation(),
                                    "class can only extend a class type");
                std::exit(1);
            }

            baseType = static_cast<ClassType*>(type);
        } else {
            if (mScope->findModuleScope()->name == "std/Primitives" && name == "Object") baseType = nullptr;
            else baseType = static_cast<ClassType*>(Type::Get("object"));
        }

        ClassType::Create(mScope->findModuleScope()->name, name, baseType);

        if (current().getTokenType() == lexer::TokenType::Semicolon) { // declaration
            consume();
            mScope->createClass(std::move(name), baseType == nullptr ? nullptr : mScope->findClass(
                    { std::string(baseType->getModuleName()), std::string(baseType->getName()) }),{}, {}, {}, false);
            return nullptr;
        }

        expectToken(lexer::TokenType::LeftBrace);
        consume();

        auto scope = std::make_unique<symbol::Scope>(mScope, name, true);
        mScope = scope.get();

        std::vector<ClassField> fields;
        std::vector<ClassMethod> constructors;
        std::vector<ClassMethod> methods;

        while (current().getTokenType() != lexer::TokenType::RightBrace) {
            modifierTokens.clear();

            while (IsModifierToken(current())) {
                modifierTokens.push_back(consume());
            }

            parseClassMember(name, fields, constructors, methods, modifierTokens);
        }
        consume();

        mScope = scope->parent;

        return std::make_unique<ClassDeclaration>(std::move(modifiers), std::move(name), std::move(fields), std::move(constructors), std::move(methods), std::move(scope), std::move(token));
    }

    void Parser::parseClassMember(std::string_view className, std::vector<ClassField>& fields,std::vector<ClassMethod>& constructors,
                                  std::vector<ClassMethod>& methods, std::vector<lexer::Token> modifierTokens) {
        auto token = current();

        if (current().getTokenType() == lexer::TokenType::Identifier && current().getText() == className && peek(1).getTokenType() == lexer::TokenType::LeftParen) { // constructor
            consume();

            expectToken(lexer::TokenType::LeftParen);
            consume();

            std::vector<MethodModifier> modifiers;
            for (auto& modifierToken : modifierTokens) {
                auto modifier = GetMethodModifier(modifierToken, mDiag);
                if (std::find(modifiers.begin(), modifiers.end(), modifier) != modifiers.end()) {
                    mDiag.compilerError(modifierToken.getStartLocation(),
                                        modifierToken.getEndLocation(),
                                        std::format("duplicate method modifier: '{}{}{}'",
                                                    fmt::bold, modifierToken.getText(), fmt::defaults));
                    std::exit(1);
                }

                //if (modifier == FunctionModifier::Native) native = true;
                modifiers.push_back(modifier);
            }

            if (std::find(modifiers.begin(), modifiers.end(), MethodModifier::Public) == modifiers.end() &&
                std::find(modifiers.begin(), modifiers.end(), MethodModifier::Private) == modifiers.end() &&
                std::find(modifiers.begin(), modifiers.end(), MethodModifier::Protected) == modifiers.end()) {
                modifiers.push_back(MethodModifier::Public);
            }

            std::vector<FunctionArgument> arguments;
            std::vector<Type*> argumentTypes;

            while (current().getTokenType() != lexer::TokenType::RightParen) {
                Type* argumentType = parseType();

                expectToken(lexer::TokenType::Identifier);
                std::string argumentName = std::string(consume().getText());

                arguments.emplace_back(argumentType, std::move(argumentName));
                argumentTypes.push_back(argumentType);

                if (current().getTokenType() != lexer::TokenType::RightParen) {
                    expectToken(lexer::TokenType::Comma);
                    consume();
                }
            }
            consume();

            FunctionType* functionType = FunctionType::Create(Type::Get("void"), std::move(argumentTypes));

            symbol::ScopePtr scope = std::make_unique<symbol::Scope>(mScope, "", false, Type::Get("void"));
            scope->currentVariableIndex = 0;
            mScope = scope.get();

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

            constructors.emplace_back(std::move(modifiers), "#Init", functionType, std::move(arguments), std::move(body), std::move(scope), std::move(token), false, false, false);
            return;
        }

        Type* type = parseType();

        std::string name = std::string(consume().getText());

        if (current().getTokenType() == lexer::TokenType::LeftParen) { // method
            consume();

            std::vector<MethodModifier> modifiers;
            bool abstract = false;
            for (auto& modifierToken : modifierTokens) {
                auto modifier = GetMethodModifier(modifierToken, mDiag);
                if (std::find(modifiers.begin(), modifiers.end(), modifier) != modifiers.end()) {
                    mDiag.compilerError(modifierToken.getStartLocation(),
                                        modifierToken.getEndLocation(),
                                        std::format("duplicate method modifier: '{}{}{}'",
                                                    fmt::bold, modifierToken.getText(), fmt::defaults));
                    std::exit(1);
                }

                if (modifier == MethodModifier::Abstract) abstract = true;

                modifiers.push_back(modifier);
            }

            if (std::find(modifiers.begin(), modifiers.end(), MethodModifier::Public) == modifiers.end() &&
                std::find(modifiers.begin(), modifiers.end(), MethodModifier::Private) == modifiers.end() &&
                std::find(modifiers.begin(), modifiers.end(), MethodModifier::Protected) == modifiers.end()) {
                modifiers.push_back(MethodModifier::Public);
            }

            std::vector<FunctionArgument> arguments;
            std::vector<Type*> argumentTypes;

            while (current().getTokenType() != lexer::TokenType::RightParen) {
                Type* argumentType = parseType();

                expectToken(lexer::TokenType::Identifier);
                std::string argumentName = std::string(consume().getText());

                arguments.emplace_back(argumentType, std::move(argumentName));
                argumentTypes.push_back(argumentType);

                if (current().getTokenType() != lexer::TokenType::RightParen) {
                    expectToken(lexer::TokenType::Comma);
                    consume();
                }
            }
            consume();

            bool overrides = false;
            bool view = false;

            while (true) {
                auto tokenType = current().getTokenType();

                if (tokenType == lexer::TokenType::OverrideKeyword) {
                    overrides = true;
                    consume();
                } else if (tokenType == lexer::TokenType::ViewKeyword) {
                    view = true;
                    consume();
                } else {
                    break;
                }
            }

            FunctionType* functionType = FunctionType::Create(type, std::move(argumentTypes));

            symbol::ScopePtr scope = std::make_unique<symbol::Scope>(mScope, "", false, type);
            scope->currentVariableIndex = 0;
            mScope = scope.get();

            if (abstract) {
                expectToken(lexer::TokenType::Semicolon);
                consume();

                methods.emplace_back(std::move(modifiers), std::move(name), functionType, std::move(arguments), std::vector<ASTNodePtr>(), std::move(scope), std::move(token), overrides, view, true);
                return;
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

            if (mImporter) {
                body.clear();
            }

            methods.emplace_back(std::move(modifiers), std::move(name), functionType, std::move(arguments), std::move(body), std::move(scope), std::move(token), overrides, view, std::find(modifiers.begin(), modifiers.end(), MethodModifier::Virtual) != modifiers.end());
        } else { // field
            std::vector<FieldModifier> modifiers;
            for (auto& modifierToken : modifierTokens) {
                auto modifier = GetFieldModifier(modifierToken, mDiag);
                if (std::find(modifiers.begin(), modifiers.end(), modifier) != modifiers.end()) {
                    mDiag.compilerError(modifierToken.getStartLocation(),
                                        modifierToken.getEndLocation(),
                                        std::format("duplicate field modifier: '{}{}{}'",
                                                    fmt::bold, modifierToken.getText(), fmt::defaults));
                    std::exit(1);
                }

                modifiers.push_back(modifier);
            }

            if (std::find(modifiers.begin(), modifiers.end(), FieldModifier::Public) == modifiers.end() &&
                std::find(modifiers.begin(), modifiers.end(), FieldModifier::Private) == modifiers.end() &&
                std::find(modifiers.begin(), modifiers.end(), FieldModifier::Protected) == modifiers.end()) {
                modifiers.push_back(FieldModifier::Public);
            }

            //TODO: initial value

            expectToken(lexer::TokenType::Semicolon);
            consume();

            fields.emplace_back(std::move(modifiers), type, std::move(name));
        }
    }

    GlobalVarPtr Parser::parseGlobalVar(std::vector<lexer::Token> modifierTokens, Type* type) {
        std::vector<GlobalVarModifier> modifiers;
        modifiers.reserve(modifierTokens.size());

        for (const auto& token : modifierTokens) {
            modifiers.push_back(GetGlobalVarModifier(token, mDiag));
        }

        expectToken(lexer::TokenType::Identifier);
        auto token = current();
        std::string name = std::string(consume().getText());

        ASTNodePtr initialValue = nullptr;

        if (current().getTokenType() == lexer::TokenType::Equal) {
            consume();
            initialValue = parseExpression();

            if (type == nullptr) type = initialValue->getType();
        }

        expectToken(lexer::TokenType::Semicolon);
        consume();

        return std::make_unique<GlobalVar>(std::move(modifiers), mScope, type, std::move(name), std::move(initialValue), std::move(token));
    }

    InitBlockPtr Parser::parseInitBlock() {
        auto token = current();
        consume(); // {

        symbol::ScopePtr scope = std::make_unique<symbol::Scope>(mScope, "", false, Type::Get("void"));
        scope->currentVariableIndex = 0;
        mScope = scope.get();

        std::vector<ASTNodePtr> body;
        while (current().getTokenType() != lexer::TokenType::RightBrace) {
            body.push_back(parseExpression());
            expectToken(lexer::TokenType::Semicolon);
            consume();
        }
        consume();

        mScope = scope->parent;

        return std::make_unique<InitBlock>(std::move(scope), std::move(body), std::move(token));
    }

    ImportStatementPtr Parser::parseImport() {
        auto token = consume();

        std::vector<std::string> module;
        while (current().getTokenType() != lexer::TokenType::Semicolon) {
            expectToken(lexer::TokenType::Identifier);
            module.emplace_back(consume().getText());

            if (current().getTokenType() != lexer::TokenType::Semicolon) {
                expectToken(lexer::TokenType::Dot);
                consume();
            }
        }
        consume();

        return std::make_unique<ImportStatement>(mScope, std::move(module), std::move(token));
    }

    ASTNodePtr Parser::parseParenExpression() {
        consume(); // (
        auto expr = parseExpression();
        expectToken(lexer::TokenType::RightParen);
        consume();

        return expr;
    }

    IntegerLiteralPtr Parser::parseIntegerLiteral() {
        auto token = consume();
        std::string text = std::string(token.getText());

        return std::make_unique<IntegerLiteral>(mScope, std::strtoimax(text.c_str(), nullptr, 0), Type::Get("int"), std::move(token));
    }

    IntegerLiteralPtr Parser::parseCharacterLiteral() {
        auto token = consume();
        char value = token.getText()[0];
        return std::make_unique<IntegerLiteral>(mScope, value, Type::Get("char"), std::move(token));
    }

    StringLiteralPtr Parser::parseStringLiteral() {
        auto token = consume();
        std::string text = std::string(token.getText());

        return std::make_unique<StringLiteral>(mScope, std::move(text), std::move(token));
    }

    MemberAccessPtr Parser::parseMemberAccess(ASTNodePtr classNode) {
        expectToken(lexer::TokenType::Identifier);
        auto token = consume();
        std::string text = std::string(token.getText());

        return std::make_unique<MemberAccess>(mScope, std::move(classNode), std::move(text), peek(-2), std::move(token));
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

    ASTNodePtr Parser::parseNewExpression() {
        auto token = consume(); // new

        Type* type = parseType(false, true);

        if (current().getTokenType() == lexer::TokenType::LeftBracket)  {
            consume();

            ASTNodePtr length = parseExpression();

            expectToken(lexer::TokenType::RightBracket);
            consume();

            return std::make_unique<NewArrayExpression>(mScope, ArrayType::Create(type), std::move(length), std::move(token));
        }

        //TODO: unsafe new without constructor call

        expectToken(lexer::TokenType::LeftParen);
        consume();

        std::vector<ASTNodePtr> parameters;
        while (current().getTokenType() != lexer::TokenType::RightParen) {
            parameters.push_back(parseExpression());
            if (current().getTokenType() != lexer::TokenType::RightParen) {
                expectToken(lexer::TokenType::Comma);
                consume();
            }
        }
        consume();

        return std::make_unique<NewExpression>(mScope, type, std::move(parameters), std::move(token));
    }

    CompoundStatementPtr Parser::parseCompoundStatement() {
        auto token = consume(); // {

        symbol::ScopePtr scope = std::make_unique<symbol::Scope>(mScope, "", false);
        mScope = scope.get();

        std::vector<ASTNodePtr> body;
        while (current().getTokenType() != lexer::TokenType::RightBrace) {
            body.push_back(parseExpression());
            expectToken(lexer::TokenType::Semicolon);
            consume();
        }
        consume();

        mTokens.insert(mTokens.begin() + mPosition, lexer::Token(";", lexer::TokenType::Semicolon, {}, {}));

        mScope = scope->parent;

        return std::make_unique<CompoundStatement>(std::move(scope), std::move(body), std::move(token));
    }

    IfStatementPtr Parser::parseIfStatement() {
        auto token = consume();

        expectToken(lexer::TokenType::LeftParen);
        consume();

        auto condition = parseExpression();

        expectToken(lexer::TokenType::RightParen);
        consume();

        symbol::ScopePtr scope = std::make_unique<symbol::Scope>(mScope, "", false);
        mScope = scope.get();

        auto body = parseExpression();
        ASTNodePtr elseBody = nullptr;

        if (peek(1).getTokenType() == lexer::TokenType::ElseKeyword) {
            expectToken(lexer::TokenType::Semicolon);
            consume();

            consume(); // else

            elseBody = parseExpression();
        }

        mScope = scope->parent;

        return std::make_unique<IfStatement>(std::move(scope) ,std::move(condition), std::move(body), std::move(elseBody), std::move(token));
    }

    WhileStatementPtr Parser::parseWhileStatement() {
        auto token = consume();

        expectToken(lexer::TokenType::LeftParen);
        consume();

        auto condition = parseExpression();

        expectToken(lexer::TokenType::RightParen);
        consume();

        std::string name;
        if (current().getTokenType() == lexer::TokenType::Hash) {
            consume();
            expectToken(lexer::TokenType::Identifier);
            name = consume().getText();
        }

        symbol::ScopePtr scope = std::make_unique<symbol::Scope>(mScope, "", false);
        mScope = scope.get();

        auto body = parseExpression();

        mScope = mScope->parent;

        return std::make_unique<WhileStatement>(std::move(scope), std::move(condition), std::move(body), std::move(name), std::move(token));
    }

    ForStatementPtr Parser::parseForStatement() {
        auto token = consume();

        expectToken(lexer::TokenType::LeftParen);
        consume();

        auto init = parseExpression();
        expectToken(lexer::TokenType::Semicolon);
        consume();

        auto condition = parseExpression();
        expectToken(lexer::TokenType::Semicolon);
        consume();

        auto it = parseExpression();

        expectToken(lexer::TokenType::RightParen);
        consume();

        std::string name;
        if (current().getTokenType() == lexer::TokenType::Hash) {
            consume();
            expectToken(lexer::TokenType::Identifier);
            name = consume().getName();
        }

        symbol::ScopePtr scope = std::make_unique<symbol::Scope>(mScope, "", false);
        mScope = scope.get();

        auto body = parseExpression();

        mScope = mScope->parent;

        return std::make_unique<ForStatement>(std::move(scope), std::move(init), std::move(condition), std::move(it), std::move(body), std::move(name), std::move(token));
    }

    BreakStatementPtr Parser::parseBreakStatement() {
        auto token = consume();

        std::string name;
        if (current().getTokenType() == lexer::TokenType::Hash) {
            consume();
            expectToken(lexer::TokenType::Identifier);
            name = consume().getText();
        }

        return std::make_unique<BreakStatement>(mScope, std::move(name), std::move(token));
    }

    ContinueStatementPtr Parser::parseContinueStatement() {
        auto token = consume();

        std::string name;
        if (current().getTokenType() == lexer::TokenType::Hash) {
            consume();
            expectToken(lexer::TokenType::Identifier);
            name = consume().getText();
        }

        return std::make_unique<ContinueStatement>(mScope, std::move(name), std::move(token));
    }

    ReturnStatementPtr Parser::parseReturnStatement() {
        auto token = consume();

        if (current().getTokenType() == lexer::TokenType::Semicolon) {
            return std::make_unique<ReturnStatement>(mScope, nullptr, std::move(token));
        }

        return std::make_unique<ReturnStatement>(mScope, parseExpression(), std::move(token));
    }

    VariableDeclarationPtr Parser::parseVariableDeclaration(Type* type) {
        expectToken(lexer::TokenType::Identifier);
        auto token = current();
        std::string name = std::string(consume().getText());

        ASTNodePtr initialValue = nullptr;

        if (current().getTokenType() == lexer::TokenType::Equal) {
            consume();
            initialValue = parseExpression();

            if (type == nullptr) type = initialValue->getType();
        }

        return std::make_unique<VariableDeclaration>(mScope, type, std::move(name), std::move(initialValue), std::move(token));
    }

    bool IsModifierToken(const lexer::Token& token) {
        static constexpr std::array modifiers = {
                lexer::TokenType::AbstractKeyword,
                lexer::TokenType::NativeKeyword,
                lexer::TokenType::PublicKeyword,
                lexer::TokenType::PrivateKeyword,
                lexer::TokenType::ProtectedKeyword,
                lexer::TokenType::VirtualKeyword,
        };

        return std::find(modifiers.begin(), modifiers.end(), token.getTokenType()) != modifiers.end();
    }

    ClassModifier GetClassModifier(const lexer::Token& token, diagnostic::Diagnostics& diag) {
        switch (token.getTokenType()) {
            case lexer::TokenType::PublicKeyword:
                return ClassModifier::Public;
            case lexer::TokenType::PrivateKeyword:
                return ClassModifier::Private;
            case lexer::TokenType::AbstractKeyword:
                return ClassModifier::Abstract;

            default:
                diag.compilerError(token.getStartLocation(),
                                   token.getEndLocation(),
                                   std::format("invalid class modifier: '{}{}{}'",
                                               fmt::bold, token.getText(), fmt::defaults));
                std::exit(1);
        }
    }

    FieldModifier GetFieldModifier(const lexer::Token& token, diagnostic::Diagnostics& diag) {
        switch (token.getTokenType()) {
            case lexer::TokenType::PublicKeyword:
                return FieldModifier::Public;
            case lexer::TokenType::PrivateKeyword:
                return FieldModifier::Private;
            case lexer::TokenType::ProtectedKeyword:
                return FieldModifier::Protected;

            default:
                diag.compilerError(token.getStartLocation(),
                                   token.getEndLocation(),
                                   std::format("invalid field modifier: '{}{}{}'",
                                               fmt::bold, token.getText(), fmt::defaults));
                std::exit(1);
        }
    }

    MethodModifier GetMethodModifier(const lexer::Token& token, diagnostic::Diagnostics& diag) {
        switch (token.getTokenType()) {
            case lexer::TokenType::PublicKeyword:
                return MethodModifier::Public;
            case lexer::TokenType::PrivateKeyword:
                return MethodModifier::Private;
            case lexer::TokenType::ProtectedKeyword:
                return MethodModifier::Protected;
            case lexer::TokenType::VirtualKeyword:
                return MethodModifier::Virtual;
            case lexer::TokenType::AbstractKeyword:
                return MethodModifier::Abstract;

            default:
                diag.compilerError(token.getStartLocation(),
                                   token.getEndLocation(),
                                   std::format("invalid method modifier: '{}{}{}'",
                                               fmt::bold, token.getText(), fmt::defaults));
            std::exit(1);
        }
    }

    FunctionModifier GetFunctionModifier(const lexer::Token& token, diagnostic::Diagnostics& diag) {
        switch (token.getTokenType()) {
            case lexer::TokenType::NativeKeyword:
                return FunctionModifier::Native;
            case lexer::TokenType::PublicKeyword:
                return FunctionModifier::Public;
            case lexer::TokenType::PrivateKeyword:
                return FunctionModifier::Private;

            default:
                diag.compilerError(token.getStartLocation(),
                                   token.getEndLocation(),
                                   std::format("invalid function modifier: '{}{}{}'",
                                               fmt::bold, token.getText(), fmt::defaults));
                std::exit(1);
        }
    }

    GlobalVarModifier GetGlobalVarModifier(const lexer::Token& token, diagnostic::Diagnostics& diag) {
        switch (token.getTokenType()) {
            case lexer::TokenType::PublicKeyword:
                return GlobalVarModifier::Public;
            case lexer::TokenType::PrivateKeyword:
                return GlobalVarModifier::Private;

            default:
                diag.compilerError(token.getStartLocation(),
                                   token.getEndLocation(),
                                   std::format("invalid global var modifier: '{}{}{}'",
                                               fmt::bold, token.getText(), fmt::defaults));
                std::exit(1);
        }
    }
}
