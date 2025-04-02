// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ImportParser.h"
#include "Bibble/parser/Parser.h"

#include <algorithm>

namespace parser {
    ImportParser::ImportParser(std::vector<lexer::Token>& tokens, diagnostic::Diagnostics& diag,
                               symbol::ImportManager& importManager, symbol::Scope* globalScope)
                               : mTokens(tokens)
                               , mPosition(0)
                               , mImportManager(importManager)
                               , mScope(globalScope)
                               , mDiag(diag) {}

    std::vector<ASTNodePtr> ImportParser::parse() {
        std::vector<ASTNodePtr> ast;

        while (mPosition < mTokens.size()) {
            auto global = parseGlobal();
            if (global) {
                ast.push_back(std::move(global));
            }
        }

        return ast;
    }

    lexer::Token ImportParser::current() const {
        return mTokens[mPosition];
    }

    lexer::Token ImportParser::consume() {
        return mTokens[mPosition++];
    }

    lexer::Token ImportParser::peek(int offset) const {
        return mTokens[mPosition + offset];
    }

    void ImportParser::expectToken(lexer::TokenType type) {
        if (current().getTokenType() != type) {
            lexer::Token temp("", type, lexer::SourceLocation(), lexer::SourceLocation());
            mDiag.compilerError(current().getStartLocation(),
                                current().getEndLocation(),
                                std::format("expected '{}{}{}', found '{}{}{}'",
                                            fmt::bold, temp.getName(), fmt::defaults,
                                            fmt::bold, current().getText(), fmt::defaults));
            std::exit(EXIT_FAILURE);
        }
    }

    Type* ImportParser::parseType() {
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

    ASTNodePtr ImportParser::parseGlobal() {
        auto start = current().getStartLocation();

        std::vector<lexer::Token> modifierTokens;
        while (IsModifierToken(current())) {
            modifierTokens.push_back(consume());
        }

        switch (current().getTokenType()) {
            case lexer::TokenType::ImportKeyword:
                if (!modifierTokens.empty()) {
                    mDiag.compilerError(start, current().getEndLocation(), "didn't expect modifiers here");
                }

                parseImport();
                return nullptr;

            case lexer::TokenType::Identifier:
            case lexer::TokenType::Type:
                return parseFunction(std::move(modifierTokens));

            case lexer::TokenType::EndOfFile:
                consume();
                return nullptr;

            default:
                mDiag.compilerError(current().getStartLocation(),
                                    current().getEndLocation(),
                                    std::format("expected global expression. Found '{}{}{}'",
                                                fmt::bold, current().getText(), fmt::defaults));
                std::exit(EXIT_FAILURE);
                return nullptr;
        }
    }

    void ImportParser::parseImport() {
        consume(); // import

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

        std::string shortModuleName;
        auto pos = moduleName.find_last_of('/');

        if (pos == std::string::npos) {
            shortModuleName = moduleName;
        } else {
            shortModuleName = moduleName.substr(pos + 1);
        }

        mScope->getTopLevelScope()->importedModuleNames[std::move(shortModuleName)] = std::move(moduleName);
    }

    FunctionPtr ImportParser::parseFunction(std::vector<lexer::Token> modifierTokens) {
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

        int braceDepth = 1;
        while (braceDepth > 0) {
            auto type = current().getTokenType();
            if (type == lexer::TokenType::LeftBrace) {
                braceDepth++;
            } else if (type == lexer::TokenType::RightBrace) {
                braceDepth--;
            }

            consume();
        }

        mScope = scope->parent;

        return std::make_unique<Function>(std::move(modifiers), std::move(name), functionType, std::move(arguments), std::vector<ASTNodePtr>(), std::move(scope), std::move(token));
    }

    ClassDeclarationPtr ImportParser::parseClass(std::vector<lexer::Token> modifierTokens) {
        return nullptr;
    }

    void ImportParser::parseClassMember(std::vector<ClassField>& fields, std::vector<ClassMethod>& methods, std::vector<lexer::Token> modifierTokens) {

    }
}