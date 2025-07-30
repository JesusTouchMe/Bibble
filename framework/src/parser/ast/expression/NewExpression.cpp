// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/expression/NewExpression.h"

#include <algorithm>
#include <format>

namespace parser {
    NewExpression::NewExpression(symbol::Scope* scope, Type* type, std::vector<ASTNodePtr> parameters, lexer::Token token)
        : ASTNode(scope, type, std::move(token))
        , mParameters(std::move(parameters))
        , mBestViableConstructor(nullptr) {}

    void NewExpression::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag,
                                bool statement) {
        builder.createNew(mType);
        if (!statement) {
            builder.createDup(mType); // we're gonna only dup it if we plan on keeping the reference otherwise we might as well let it die after the constructor call
        }

        for (auto& parameter : mParameters) {
            parameter->codegen(builder, ctx, diag, false);
        }

        builder.createCall(mBestViableConstructor->moduleName, mBestViableConstructor->name, mBestViableConstructor->type);
    }

    void NewExpression::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        for (auto& parameter : mParameters) {
            parameter->semanticCheck(diag, exit, statement);
        }
    }

    void NewExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        for (auto& parameter : mParameters) {
            parameter->typeCheck(diag, exit);
        }

        if (!mType->isClassType()) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               std::format("'{}operator new{}' used on non-class type '{}{}{}'",
                                           fmt::bold, fmt::defaults,
                                           fmt::bold, mType->getName(), fmt::defaults));
            exit = true;
            return;
        }

        auto classType = static_cast<ClassType*>(mType);

        mBestViableConstructor = getBestViableConstructor(diag, exit);

        if (mBestViableConstructor == nullptr) {
            exit = true;
        } else {
            auto functionType = mBestViableConstructor->type;
            u16 index = 1;
            for (auto& parameter : mParameters) {
                auto argumentType = functionType->getArgumentTypes()[index++];
                if (parameter->getType() != argumentType) {
                    if (parameter->implicitCast(diag, argumentType)) {
                        parameter = Cast(parameter, argumentType);
                    } else {
                        diag.compilerError(mErrorToken.getStartLocation(),
                                           mErrorToken.getEndLocation(),
                                           std::format("no matching function for call to '{}{}::{}(){}'",
                                                       fmt::bold, classType->getName(), classType->getName(), fmt::defaults));

                        exit = true;
                    }
                }
            }
        }
    }

    bool NewExpression::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }

    struct ViableFunction {
        symbol::FunctionSymbol* symbol;
        int score;
        bool disallowed;
    };

    symbol::FunctionSymbol* NewExpression::getBestViableConstructor(diagnostic::Diagnostics& diag, bool& exit) {
        std::vector<symbol::FunctionSymbol*> candidateConstructors;
        auto classType = static_cast<ClassType*>(mType);
        symbol::ClassSymbol* classSymbol = mScope->findClass({ std::string(classType->getModuleName()), std::string(classType->getName()) });

        for (const auto& constructor : classSymbol->constructors) {
            candidateConstructors.push_back(constructor.function);
        }

        for (auto it = candidateConstructors.begin(); it != candidateConstructors.end();) {
            auto candidate = *it;
            auto& arguments = candidate->type->getArgumentTypes();

            if (arguments.size() != mParameters.size() + 1) {
                candidateConstructors.erase(it);
            } else {
                ++it;
            }
        }

        std::vector<ViableFunction> viableFunctions;
        for (auto* candidate : candidateConstructors) {
            int score = 0;
            bool disallowed = false;

            for (size_t i = 0; i < mParameters.size(); i++) {
                Type::CastLevel castLevel = mParameters[i]->getType()->castTo(candidate->type->getArgumentTypes()[i + 1]);
                int multiplier = 0;

                if (mParameters[i]->getType() == candidate->type->getArgumentTypes()[i]) multiplier = 0;
                else if (castLevel == Type::CastLevel::Implicit) multiplier = 1;
                else if (castLevel == Type::CastLevel::ImplicitWarning) multiplier = 2;
                else disallowed = true;

                score += multiplier * (mParameters.size() - i);
            }

            if (!disallowed) {
                viableFunctions.push_back({ candidate, score, false });
            }
        }

        if (viableFunctions.empty()) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               std::format("no matching function for call to '{}{}::{}(){}'",
                                           fmt::bold, classType->getName(), classType->getName(), fmt::defaults));
            return nullptr;
        }

        std::sort(viableFunctions.begin(), viableFunctions.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.score < rhs.score;
        });

        if (viableFunctions.size() >= 2) {
            if (viableFunctions[0].score == viableFunctions[1].score) {
                diag.compilerError(mErrorToken.getStartLocation(),
                                   mErrorToken.getEndLocation(),
                                   std::format("call to '{}{}::{}(){}' is ambiguous",
                                               fmt::bold, classType->getName(), classType->getName(), fmt::defaults));
                return nullptr;
            }
        }

        return viableFunctions.front().symbol;
    }
}