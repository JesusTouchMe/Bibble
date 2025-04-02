// Copyright 2025 JesusTouchMe

#include "Bible/parser/ast/expression/CallExpression.h"
#include "Bible/parser/ast/expression/VariableExpression.h"

#include <algorithm>
#include <format>

namespace parser {
    CallExpression::CallExpression(symbol::Scope* scope, ASTNodePtr callee, std::vector<ASTNodePtr> parameters)
        : ASTNode(scope, callee->getErrorToken())
        , mCallee(std::move(callee))
        , mParameters(std::move(parameters))
        , mBestViableFunction(nullptr)
        , mIsMemberFunction(false) {}

    void CallExpression::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag) {
        if (mBestViableFunction == nullptr) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               "no appropriate function was found for call");
            std::exit(1);
        }

        if (mIsMemberFunction) {
            if (auto var = dynamic_cast<VariableExpression*>(mCallee.get())) {
                symbol::LocalSymbol* self = mScope->findLocal("this");
                builder.createLoad(self->type, self->index);
            } else {
                //TODO: member access
            }
        }

        for (auto& param : mParameters) {
            param->codegen(builder,ctx, diag);
        }

        builder.createCall(mBestViableFunction->moduleName, mBestViableFunction->name, mBestViableFunction->type);
    }

    void CallExpression::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        mCallee->semanticCheck(diag, exit, false);
        for (auto& parameter : mParameters) {
            parameter->semanticCheck(diag, exit, false);
        }
    }

    void CallExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        mCallee->typeCheck(diag, exit);
        for (auto& parameter : mParameters) {
            parameter->typeCheck(diag, exit);
        }

        mBestViableFunction = getBestViableFunction(diag, exit);

        if (mBestViableFunction == nullptr) {
            exit = true;
        } else {
            auto functionType = mBestViableFunction->type;
            mType = functionType->getReturnType();
            u16 index = 0;
            for (auto& parameter : mParameters) {
                auto argumentType = functionType->getArgumentTypes()[index];
                if (parameter->getType() != argumentType) {
                    if (parameter->implicitCast(diag, argumentType)) {
                        parameter = Cast(parameter, argumentType);
                    } else {
                        diag.compilerError(mErrorToken.getStartLocation(),
                                           mErrorToken.getEndLocation(),
                                           std::format("no matching function for call to '{}{}.{}(){}'",
                                                       fmt::bold, mScope->getTopLevelScope()->name, mBestViableFunction->name, fmt::defaults));

                        exit = true;
                    }
                }
            }
        }
    }

    bool CallExpression::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) {
        return false;
    }

    struct ViableFunction {
        symbol::FunctionSymbol* symbol;
        int score;
        bool disallowed;
    };

    // million social credits to solar mist for making this code before i stole it
    symbol::FunctionSymbol* CallExpression::getBestViableFunction(diagnostic::Diagnostics& diag, bool& exit) {
        if (dynamic_cast<VariableExpression*>(mCallee.get())) { // || MemberAccess*
            std::vector<symbol::FunctionSymbol*> candidateFunctions;
            std::string errorName;

            if (auto var = dynamic_cast<VariableExpression*>(mCallee.get())) {
                errorName = var->getName();

                if (var->isImplicitMember()) {
                    diag.fatalError("unimplemented");
                } else {
                    candidateFunctions = mScope->getCandidateFunctions(var->getNames());
                }
            }

            for (auto it = candidateFunctions.begin(); it != candidateFunctions.end();) {
                auto candidate = *it;
                auto arguments = candidate->type->getArgumentTypes();

                if (mIsMemberFunction) {
                    if (arguments.size() != mParameters.size() + 1) {
                        candidateFunctions.erase(it);
                    } else {
                        ++it;
                    }
                } else {
                    if (arguments.size() != mParameters.size()) it = candidateFunctions.erase(it);
                    else ++it;
                }
            }

            std::vector<ViableFunction> viableFunctions;
            for (auto& candidate : candidateFunctions) {
                int score = 0;
                bool disallowed = false;
                size_t i = mIsMemberFunction ? 1 : 0;

                for (; i < mParameters.size(); i++) {
                    Type::CastLevel castLevel = mParameters[i]->getType()->castTo(candidate->type->getArgumentTypes()[i]);
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
                                   std::format("no matching function for call to '{}{}(){}'",
                                               fmt::bold, errorName, fmt::defaults));
                return nullptr;
            }

            std::sort(viableFunctions.begin(), viableFunctions.end(), [](const auto& lhs, const auto& rhs) {
                return lhs.score < rhs.score;
            });

            if (viableFunctions.size() >= 2) {
                if (viableFunctions[0].score == viableFunctions[1].score) {
                    diag.compilerError(mErrorToken.getStartLocation(),
                                       mErrorToken.getEndLocation(),
                                       std::format("call to '{}{}(){}' is ambiguous",
                                                   fmt::bold, errorName, fmt::defaults));
                    return nullptr;
                }
            }

            return viableFunctions.front().symbol;
        }

        return nullptr;
    }
}