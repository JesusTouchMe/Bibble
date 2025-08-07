// Copyright 2025 JesusTouchMe

#include "Bibble/parser/ast/expression/CallExpression.h"
#include "Bibble/parser/ast/expression/MemberAccess.h"
#include "Bibble/parser/ast/expression/VariableExpression.h"

#include "Bibble/type/ViewType.h"

#include <algorithm>
#include <format>

#include "Bibble/parser/ast/expression/Integerliteral.h"
#include "Bibble/parser/ast/expression/StringLiteral.h"

namespace parser {
    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

    constexpr bool operator==(const CallExpression::FunctionOrMethod& func, std::nullptr_t) {
        return std::holds_alternative<std::monostate>(func);
    }

    constexpr std::string_view GetModuleName(const CallExpression::FunctionOrMethod& func) {
        return std::visit(overloaded{
            [](const std::monostate&) -> std::string_view { return "<error>"; },
            [](const symbol::FunctionSymbol* f) -> std::string_view { return f->moduleName; },
            [](const symbol::ClassSymbol::Method* m) -> std::string_view { return m->function->moduleName; }
        }, func);
    }

    constexpr std::string_view GetName(const CallExpression::FunctionOrMethod& func) {
        return std::visit(overloaded{
        [](const std::monostate&) -> std::string_view { return "<error>"; },
        [](const symbol::FunctionSymbol* f) -> std::string_view { return f->name; },
        [](const symbol::ClassSymbol::Method* m) -> std::string_view { return m->name; }
        }, func);
    }

    constexpr u16 GetModifiers(const CallExpression::FunctionOrMethod& func) {
        return std::visit(overloaded{
            [](const std::monostate&) -> u16 { return 0; },
            [](const symbol::FunctionSymbol* f) { return f->modifiers; },
            [](const symbol::ClassSymbol::Method* m) { return m->modifiers; }
        }, func);
    }

    constexpr FunctionType* GetType(const CallExpression::FunctionOrMethod& func) {
        return std::visit(overloaded{
            [](const std::monostate&) -> FunctionType* { return nullptr; },
            [](const symbol::FunctionSymbol* f) { return f->type; },
            [](const symbol::ClassSymbol::Method* m) { return m->type; }
        }, func);
    }

    constexpr bool AppendCallerLocation(const CallExpression::FunctionOrMethod& func) {
        return std::visit(overloaded{
            [](const std::monostate&) -> bool { return false; },
            [](const symbol::FunctionSymbol* f) -> bool { return f->appendCallerLocation; },
            [](const symbol::ClassSymbol::Method* m) -> bool { return false; }
        }, func);
    }

    CallExpression::CallExpression(symbol::Scope* scope, ASTNodePtr callee, std::vector<ASTNodePtr> parameters)
        : ASTNode(scope, callee->getErrorToken())
        , mCallee(std::move(callee))
        , mParameters(std::move(parameters))
        , mIsMemberFunction(false) {}

    void CallExpression::codegen(codegen::Builder& builder, codegen::Context& ctx, diagnostic::Diagnostics& diag, bool statement) {
        if (mBestViableFunction == nullptr) {
            diag.compilerError(mErrorToken.getStartLocation(),
                               mErrorToken.getEndLocation(),
                               "no appropriate function was found for call");
            std::exit(1);
        }

        ClassType* classType = nullptr;

        if (mIsMemberFunction) {
            if (auto var = dynamic_cast<VariableExpression*>(mCallee.get())) {
                symbol::LocalSymbol* self = mScope->findLocal("this");
                builder.createLoad(self->type, self->index);

                if (self->type->isViewType()) {
                    classType = static_cast<ClassType*>(static_cast<ViewType*>(self->type)->getBaseType());
                } else {
                    classType = static_cast<ClassType*>(self->type);
                }
            } else {
                auto member = static_cast<MemberAccess*>(mCallee.get());
                member->mClass->codegen(builder, ctx, diag, false);

                classType = member->mClassType;
            }
        }

        for (auto& parameter : mParameters) {
            parameter->codegen(builder, ctx, diag, false);
        }

        std::visit(overloaded{
            [](const std::monostate&) { },
            [&builder](const symbol::FunctionSymbol* f) {
                builder.createCall(f->moduleName, f->name, f->type);
            },
            [&builder, classType](const symbol::ClassSymbol::Method* m) {
                if (m->isVirtual) {
                    builder.createVirtualCall(classType, m->name, m->type);
                } else {
                    builder.createCall(m->function->moduleName, m->function->name, m->function->type);
                }
            }
        }, mBestViableFunction);

        if (statement && !mType->isVoidType()) {
            builder.createPop(mType);
        }
    }

    void CallExpression::semanticCheck(diagnostic::Diagnostics& diag, bool& exit, bool statement) {
        mCallee->semanticCheck(diag, exit, false);
        for (auto& parameter : mParameters) {
            parameter->semanticCheck(diag, exit, false);
        }

        if (statement && (GetModifiers(mBestViableFunction) & MODULEWEB_FUNCTION_MODIFIER_PURE)) {
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
            auto functionType = GetType(mBestViableFunction);
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
                                                       fmt::bold, GetModuleName(mBestViableFunction), GetName(mBestViableFunction), fmt::defaults));

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
        CallExpression::FunctionOrMethod symbol;
        int score;
        bool disallowed;
    };

    // million social credits to solar mist for making this code before i stole it
    CallExpression::FunctionOrMethod CallExpression::getBestViableFunction(diagnostic::Diagnostics& diag, bool& exit) {
        if (dynamic_cast<VariableExpression*>(mCallee.get()) || dynamic_cast<MemberAccess*>(mCallee.get())) {
            std::vector<FunctionOrMethod> candidateFunctions;
            std::string errorName;

            if (auto var = dynamic_cast<VariableExpression*>(mCallee.get())) {
                errorName = var->getName();

                if (var->isImplicitMember()) {
                    auto scopeOwner = mScope->findOwner();

                    symbol::LocalSymbol* localThis = mScope->findLocal("this");
                    if (localThis == nullptr) {
                        diag.fatalError("scope is owned by a class, but no 'this' local exists");
                    }

                    bool view = localThis->type->isViewType();

                    if (!view) {
                        auto candidates = scopeOwner->getCandidateMethods(var->getName());
                        candidateFunctions.insert(candidateFunctions.end(), candidates.begin(), candidates.end());
                    }

                    auto candidates = scopeOwner->getCandidateMethods(std::string(var->getName()) + ".v");
                    candidateFunctions.insert(candidateFunctions.end(), candidates.begin(), candidates.end());

                    errorName = scopeOwner->name;
                    errorName += "::" + var->getName();

                    mIsMemberFunction = true;
                } else {
                    auto candidates = mScope->getCandidateFunctions(var->getNames());
                    candidateFunctions.insert(candidateFunctions.end(), candidates.begin(), candidates.end());
                }
            } else if (auto memberAccess = dynamic_cast<MemberAccess*>(mCallee.get())) {
                ClassType* classType = memberAccess->mClassType;
                symbol::ClassSymbol* classSymbol = mScope->findClass({ std::string(classType->getModuleName()), std::string(classType->getName()) });

                bool view = memberAccess->mClass->getType()->isViewType();

                if (!view) {
                    auto candidates = classSymbol->getCandidateMethods(memberAccess->getId());
                    candidateFunctions.insert(candidateFunctions.end(), candidates.begin(), candidates.end());
                }

                auto candidates = classSymbol->getCandidateMethods(std::string(memberAccess->getId()) + ".v");
                candidateFunctions.insert(candidateFunctions.end(), candidates.begin(), candidates.end());

                errorName = classType->getName();
                errorName += "::" + memberAccess->mId;

                mIsMemberFunction = true;
            }

            for (auto it = candidateFunctions.begin(); it != candidateFunctions.end();) {
                auto candidate = *it;
                auto& arguments = GetType(candidate)->getArgumentTypes();

                int appendCallerLocation = AppendCallerLocation(candidate) ? 3 : 0;

                if (mIsMemberFunction) {
                    if (arguments.size() != mParameters.size() + 1 + appendCallerLocation) {
                        it = candidateFunctions.erase(it);
                    } else {
                        ++it;
                    }
                } else {
                    if (arguments.size() != mParameters.size() + appendCallerLocation) it = candidateFunctions.erase(it);
                    else ++it;
                }
            }

            std::vector<ViableFunction> viableFunctions;
            for (auto& candidate : candidateFunctions) {
                int score = 0;
                bool disallowed = false;
                size_t i = 0;

                for (; i < mParameters.size(); i++) {
                    Type::CastLevel castLevel = mParameters[i]->getType()->castTo(GetType(candidate)->getArgumentTypes()[i + mIsMemberFunction]);
                    int multiplier = 0;

                    if (mParameters[i]->getType() == GetType(candidate)->getArgumentTypes()[i + mIsMemberFunction]) multiplier = 0;
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
                return std::monostate{};
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
                    return std::monostate{};
                }
            }

            auto& best = viableFunctions.front().symbol;
            if (AppendCallerLocation(best)) {
                mParameters.push_back(std::make_unique<StringLiteral>(mScope, std::string(mErrorToken.getStartLocation().file), mErrorToken));
                mParameters.push_back(std::make_unique<IntegerLiteral>(mScope, mErrorToken.getStartLocation().line, Type::Get("int"), mErrorToken));
                mParameters.push_back(std::make_unique<IntegerLiteral>(mScope, mErrorToken.getStartLocation().col, Type::Get("int"), mErrorToken));
            }

            return best;
        }

        return std::monostate{};
    }
}
