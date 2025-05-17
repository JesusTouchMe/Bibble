// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/expression/CallExpression.h"
#include "Bibble/parser/ast/expression/MemberAccess.h"
#include "Bibble/parser/ast/expression/VariableExpression.h"

#include <algorithm>
#include <format>

namespace parser {
    CallExpression::CallExpression(symbol::Scope* scope, ASTNodePtr callee, std::vector<ASTNodePtr> parameters)
        : ASTNode(scope, callee->getErrorToken())
        , mCallee(std::move(callee))
        , mParameters(std::move(parameters))
        , mBestViableFunction(nullptr)
        , mIsMemberFunction(false) {}

    void CallExpression::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) {
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
                auto member = static_cast<MemberAccess*>(mCallee.get());
                member->mClass->codegen(builder, ctx, diag, false);
            }
        }

        for (auto& parameter : mParameters) {
            parameter->codegen(builder, ctx, diag, false);
        }

        builder.createCall(mBestViableFunction->moduleName, mBestViableFunction->name, mBestViableFunction->type);

        if (statement && !mType->isVoidType()) {
            builder.createPop(mType);
        }
    }

    void CallExpression::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        mCallee->semanticCheck(diag, exit, false);
        for (auto& parameter : mParameters) {
            parameter->semanticCheck(diag, exit, false);
        }

        if (statement && (mBestViableFunction->modifiers & MODULEWEB_FUNCTION_MODIFIER_PURE)) {
            mType = Type::Get("void");
            diag.compilerWarning("unused-value",
                                 mErrorToken.getStartLocation(),
                                 mErrorToken.getEndLocation(),
                                 "return value of pure function is ignored");
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
            u16 index = mIsMemberFunction ? 1 : 0;
            for (auto& parameter : mParameters) {
                auto argumentType = functionType->getArgumentTypes()[index++];
                if (parameter->getType() != argumentType) {
                    if (parameter->implicitCast(diag, argumentType)) {
                        parameter = Cast(parameter, argumentType);
                    } else {
                        diag.compilerError(mErrorToken.getStartLocation(),
                                           mErrorToken.getEndLocation(),
                                           std::format("no matching function for call to '{}{}::{}(){}'",
                                                       fmt::bold, mBestViableFunction->moduleName, mBestViableFunction->name, fmt::defaults));

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
        if (dynamic_cast<VariableExpression*>(mCallee.get()) || dynamic_cast<MemberAccess*>(mCallee.get())) {
            std::vector<symbol::FunctionSymbol*> candidateFunctions;
            std::string errorName;

            if (auto var = dynamic_cast<VariableExpression*>(mCallee.get())) {
                errorName = var->getName();

                if (var->isImplicitMember()) {
                    auto scopeOwner = mScope->findOwner();
                    ClassType* classType = scopeOwner->getType();

                    std::vector<std::string> names = {
                            std::string(classType->getModuleName()),
                            std::string(classType->getName()),
                            var->getName()
                    };

                    candidateFunctions = mScope->getCandidateFunctions(names);

                    errorName = classType->getName();
                    errorName += "::" + var->getName();

                    mIsMemberFunction = true;
                } else {
                    candidateFunctions = mScope->getCandidateFunctions(var->getNames());
                }
            } else if (auto memberAccess = dynamic_cast<MemberAccess*>(mCallee.get())) {
                ClassType* classType = memberAccess->mClassType;
                std::vector<std::string> names = {
                        std::string(classType->getModuleName()),
                        std::string(classType->getName()),
                        memberAccess->mId
                };

                candidateFunctions = mScope->getCandidateFunctions(names);

                errorName = classType->getName();
                errorName += "::" + memberAccess->mId;

                mIsMemberFunction = true;
            }

            for (auto it = candidateFunctions.begin(); it != candidateFunctions.end();) {
                auto candidate = *it;
                auto& arguments = candidate->type->getArgumentTypes();

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
            for (auto* candidate : candidateFunctions) {
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