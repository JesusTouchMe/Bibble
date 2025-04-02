// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_CODEGEN_BUILDER_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_CODEGEN_BUILDER_H

#include "Bibble/codegen/Context.h"

#include "Bibble/type/FunctionType.h"

#include <JesusASM/tree/ModuleNode.h>

#include <JesusASM/tree/instructions/CallInsnNode.h>
#include <JesusASM/tree/instructions/ClassInsnNode.h>
#include <JesusASM/tree/instructions/FieldInsnNode.h>
#include <JesusASM/tree/instructions/InsnNode.h>
#include <JesusASM/tree/instructions/IntInsnNode.h>
#include <JesusASM/tree/instructions/JumpInsnNode.h>
#include <JesusASM/tree/instructions/LdcInsnNode.h>
#include <JesusASM/tree/instructions/VarInsnNode.h>

#include <JesusASM/type/Type.h>

#include <cassert>
#include <iostream>

namespace codegen {
    using Label = JesusASM::tree::LabelNode;
    using LabelPtr = std::unique_ptr<Label>;

    class Builder {
    public:
        using Opcodes = JesusASM::Opcode;

        explicit Builder(Context& ctx);

        void setInsertPoint(InsnList* insertPoint);

        ClassNode* addClass(u16 modifiers, std::string_view name, std::string_view superModule = "", std::string_view superClass = "");
        FunctionNode* addFunction(u16 modifiers, std::string_view name, JesusASM::Type* type);

        LabelPtr createLabel(std::string name);
        void insertLabel(LabelPtr label);

        void createNop();

        void createAdd(::Type* type);
        void createSub(::Type* type);
        void createMul(::Type* type);
        void createDiv(::Type* type);
        void createRem(::Type* type);
        void createAnd(::Type* type);
        void createOr(::Type* type);
        void createXor(::Type* type);
        void createShl(::Type* type);
        void createShr(::Type* type);

        void createNot(::Type* type);
        void createNeg(::Type* type);

        void createDup(::Type* type);
        void createSwap(::Type* topType, ::Type* bottomType);

        void createLoad(::Type* type, u16 index);
        void createStore(::Type* type, u16 index);

        void createArrayLoad(::Type* type);
        void createArrayStore(::Type* type);
        void createArrayLength(::Type* type);

        void createNew(::Type* type);
        void createNewArray(::Type* type);

        void createIsInstance(::Type* checkedType);

        void createGetField(::Type* ownerType, ::Type* type, std::string_view name);
        void createSetField(::Type* ownerType, ::Type* type, std::string_view name);

        void createCmp(::Type* type);

        void createJump(Label* label);
        void createCondJumpEQ(Label* trueLabel, Label* falseLabel);
        void createCondJumpNE(Label* trueLabel, Label* falseLabel);
        void createCondJumpLT(Label* trueLabel, Label* falseLabel);
        void createCondJumpGT(Label* trueLabel, Label* falseLabel);
        void createCondJumpLE(Label* trueLabel, Label* falseLabel);
        void createCondJumpGE(Label* trueLabel, Label* falseLabel);

        void createLdc(::Type* type, i64 value);
        void createLdc(::Type* type, std::nullptr_t);
        void createLdc(std::string_view value);

        void createCast(::Type* from, ::Type* to); // bytecode cast (instruction). NOT a language type cast

        void createCall(std::string_view moduleName, std::string_view name, FunctionType* type);

        void createReturn(::Type* returnType);

        void createBreakpoint();
        void createReserve1();
        void createReserve2();

    private:
        Context& mContext;
        InsnList* mInsertPoint;

        template <class T, typename... Args>
        constexpr inline T* insert(Args&&... args) {
            static_assert(std::is_constructible_v<T, Args...>,
                          "Error: Can't construct T with given args");
            mInsertPoint->add(std::make_unique<T>(std::forward<Args>(args)...));
            return static_cast<T*>(mInsertPoint->getLast());
        }

        template <JesusASM::Opcode Opcode, JesusASM::Opcode WOpcode, auto Operator>
        inline void binaryInsn(::Type* type) {
            auto rhs = mContext.pop();
            auto lhs = mContext.pop();

            assert(lhs.type == rhs.type && type->getRuntimeType() == lhs.type);

            if (lhs.value && rhs.value) {
                mInsertPoint->remove(lhs.value->origin);
                mInsertPoint->remove(rhs.value->origin);
                createLdc(type, Operator(lhs.value->value, rhs.value->value));
                return;
            }

            if (lhs.type == Type::Category1_Primitive) {
                mContext.emplace(lhs.type);
                insert<InsnNode>(Opcode);
            } else if (lhs.type == Type::Category2_Primitive) {
                mContext.emplace(lhs.type);
                insert<InsnNode>(WOpcode);
            } else {
                std::cerr << "bible: unsupported type for binary instruction template: " << type->getName() << "\n";
                std::exit(1);
            }
        }

        template <JesusASM::Opcode Opcode, JesusASM::Opcode WOpcode, auto Operator>
        inline void unaryInsn(::Type* type) {
            auto operand = mContext.pop();

            assert(operand.type == type->getRuntimeType());

            if (operand.value) {
                mInsertPoint->remove(operand.value->origin);
                createLdc(type, Operator(operand.value->value));
                return;
            }

            if (operand.type == Type::Category1_Primitive) {
                mContext.emplace(operand.type);
                insert<InsnNode>(Opcode);
            } else if (operand.type == Type::Category2_Primitive) {
                mContext.emplace(operand.type);
                insert<InsnNode>(WOpcode);
            } else {
                std::cerr << "bible: unsupported type for unary instruction template: " << type->getName() << "\n";
                std::exit(1);
            }
        }

        template <JesusASM::Opcode Opcode, auto ConstCmp>
        inline void genericCondJump(Label* trueLabel, Label* falseLabel) {
            auto condition = mContext.pop();

            assert(condition.type == Type::Category1_Primitive);

            if (condition.value) {
                mInsertPoint->remove(condition.value->origin);

                if (ConstCmp(condition.value->value)) {
                    createJump(trueLabel);
                } else {
                    createJump(falseLabel);
                }

                return;
            }

            insert<JumpInsnNode>(Opcode, trueLabel);
            insert<JumpInsnNode>(Opcodes::JMP, falseLabel);        }
    };
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_CODEGEN_BUILDER_H
