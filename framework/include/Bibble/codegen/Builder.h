// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_CODEGEN_BUILDER_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_CODEGEN_BUILDER_H

#include "Bibble/codegen/Context.h"

#include "Bibble/diagnostic/Assert.h"

#include "Bibble/type/ClassType.h"
#include "Bibble/type/FunctionType.h"

#include <JesusASM/tree/ModuleNode.h>

#include <JesusASM/tree/instructions/CallInsnNode.h>
#include <JesusASM/tree/instructions/ClassInsnNode.h>
#include <JesusASM/tree/instructions/FieldInsnNode.h>
#include <JesusASM/tree/instructions/IncInsnNode.h>
#include <JesusASM/tree/instructions/InsnNode.h>
#include <JesusASM/tree/instructions/IntInsnNode.h>
#include <JesusASM/tree/instructions/JumpInsnNode.h>
#include <JesusASM/tree/instructions/LdcInsnNode.h>
#include <JesusASM/tree/instructions/MethodInsnNode.h>
#include <JesusASM/tree/instructions/VarInsnNode.h>

#include <JesusASM/type/Type.h>

#include <iostream>

namespace Bibble {
    class Compiler;
}

namespace codegen {
    using Label = JesusASM::tree::LabelNode;
    using LabelPtr = std::unique_ptr<Label>;

    class Builder {
    friend class Bibble::Compiler;
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

        void createPop(::Type* type);

        void createDup(::Type* type);
        void createDup2(::Type* type1, ::Type* type2);
        void createDupX1(::Type* type);
        void createDupX2(::Type* type);
        void createSwap(::Type* topType, ::Type* bottomType);

        void createInc(::Type* type, u16 index, i16 increment);

        void createLoad(::Type* type, u16 index);
        void createStore(::Type* type, u16 index);

        void createArrayLoad(::Type* arrayType);
        void createArrayStore(::Type* arrayType);
        void createArrayLength(::Type* arrayType);

        void createNew(::Type* type);
        void createNewArray(::Type* arrayType);

        void createIsInstance(::Type* checkedType);

        void createGetField(::Type* ownerType, ::Type* type, std::string_view name);
        void createSetField(::Type* ownerType, ::Type* type, std::string_view name);

        void createCmpEQ(::Type* type);
        void createCmpNE(::Type* type);
        void createCmpLT(::Type* type);
        void createCmpGT(::Type* type);
        void createCmpLE(::Type* type);
        void createCmpGE(::Type* type);

        void createJump(Label* label);
        void createCondJump(Label* trueLabel, Label* falseLabel);

        void createJumpCmpEQ(::Type* type, Label* label);
        void createJumpCmpNE(::Type* type, Label* label);
        void createJumpCmpLT(::Type* type, Label* label);
        void createJumpCmpGT(::Type* type, Label* label);
        void createJumpCmpLE(::Type* type, Label* label);
        void createJumpCmpGE(::Type* type, Label* label);

        void createLdc(::Type* type, i64 value);
        void createLdc(::Type* type, bool value);
        void createLdc(::Type* type, std::nullptr_t);
        void createLdc(std::string_view value);

        void createCast(::Type* from, ::Type* to); // bytecode cast (instruction). NOT a language type cast

        void createCall(std::string_view moduleName, std::string_view name, FunctionType* type);
        void createVirtualCall(ClassType* ownerClass, std::string_view name, FunctionType* type);

        void createReturn(::Type* returnType);

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

        template <JesusASM::Opcode Opcode, auto Operator>
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

            if (lhs.type == Type::Primitive) {
                mContext.emplace(lhs.type);
                insert<InsnNode>(Opcode);
            } else {
                std::cerr << "bibble: unsupported type for binary instruction template: " << type->getName() << "\n";
                std::exit(1);
            }
        }

        template <JesusASM::Opcode Opcode, auto Operator>
        inline void unaryInsn(::Type* type) {
            auto operand = mContext.pop();

            assert(operand.type == type->getRuntimeType());

            if (operand.value) {
                mInsertPoint->remove(operand.value->origin);
                createLdc(type, Operator(operand.value->value));
                return;
            }

            if (operand.type == Type::Primitive) {
                mContext.emplace(operand.type);
                insert<InsnNode>(Opcode);
            } else {
                std::cerr << "bibble: unsupported type for unary instruction template: " << type->getName() << "\n";
                std::exit(1);
            }
        }

        template <CmpOperator Op>
        inline void cmpInsn(::Type* type) {
            auto rhs = mContext.pop();
            auto lhs = mContext.pop();

            assert(lhs.type == rhs.type && type->getRuntimeType() == lhs.type);

            if (lhs.value && rhs.value) {
                mInsertPoint->remove(lhs.value->origin);
                mInsertPoint->remove(rhs.value->origin);

                if (lhs.value->value < rhs.value->value) createLdc(::Type::Get("int"), (i64) -1);
                else if (lhs.value->value > rhs.value->value) createLdc(::Type::Get("int"), (i64) 1);
                else createLdc(::Type::Get("int"), (i64) 0);

                mContext.top().type = Type::CmpResult;
                mContext.top().cmpOperator = Op;

                return;
            }

            mContext.emplace(Op);

            switch (lhs.type) {
                case Type::Primitive:
                    insert<InsnNode>(Opcodes::CMP);
                    break;
                case Type::Handle:
                    insert<InsnNode>(Opcodes::HCMP);
                    break;
                case Type::Reference:
                    insert<InsnNode>(Opcodes::RCMP);
                    break;
                default:
                    assert(false && "bad codegen");
            }
        }
    };
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_CODEGEN_BUILDER_H
