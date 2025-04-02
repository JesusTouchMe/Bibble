// Copyright 2025 JesusTouchMe

#include "Bibble/codegen/Builder.h"

#include "Bibble/type/ClassType.h"
#include "Bibble/type/IntegerType.h"

namespace codegen {
    Builder::Builder(Context& ctx)
        : mContext(ctx)
        , mInsertPoint(nullptr) {}

    void Builder::setInsertPoint(InsnList* insertPoint) {
        mInsertPoint = insertPoint;
        mContext.mVirtualStack.clear();
    }

    ClassNode* Builder::addClass(u16 modifiers, std::string_view name, std::string_view superModule, std::string_view superClass) {
        mContext.getModule()->classes.push_back(std::make_unique<ClassNode>
                (modifiers, name, JesusASM::Name(superModule, superClass)));
        return mContext.getModule()->classes.back().get();
    }

    FunctionNode* Builder::addFunction(u16 modifiers, std::string_view name, JesusASM::Type* type) {
        mContext.getModule()->functions.push_back(std::make_unique<FunctionNode>(modifiers, name, type->getDescriptor()));
        return mContext.getModule()->functions.back().get();
    }

    LabelPtr Builder::createLabel(std::string name) {
        return std::make_unique<Label>(std::move(name));
    }

    void Builder::insertLabel(LabelPtr label) {
        mInsertPoint->add(std::move(label));
    }

    void Builder::createNop() {
        insert<InsnNode>(Opcodes::NOP);
    }

    void Builder::createAdd(::Type* type) {
        binaryInsn<Opcodes::ADD, Opcodes::LADD, [](i64 lhs, i64 rhs) { return lhs + rhs; }>(type);
    }

    void Builder::createSub(::Type* type) {
        binaryInsn<Opcodes::SUB, Opcodes::LSUB, [](i64 lhs, i64 rhs) { return lhs - rhs; }>(type);
    }

    void Builder::createMul(::Type* type) {
        binaryInsn<Opcodes::MUL, Opcodes::LMUL, [](i64 lhs, i64 rhs) { return lhs * rhs; }>(type);
    }

    void Builder::createDiv(::Type* type) {
        binaryInsn<Opcodes::DIV, Opcodes::LDIV, [](i64 lhs, i64 rhs) { return lhs / rhs; }>(type);
    }

    void Builder::createRem(::Type* type) {
        binaryInsn<Opcodes::REM, Opcodes::LREM, [](i64 lhs, i64 rhs) { return lhs % rhs; }>(type);
    }

    void Builder::createAnd(::Type* type) {
        binaryInsn<Opcodes::AND, Opcodes::LAND, [](i64 lhs, i64 rhs) { return lhs & rhs; }>(type);
    }

    void Builder::createOr(::Type* type) {
        binaryInsn<Opcodes::OR, Opcodes::LOR, [](i64 lhs, i64 rhs) { return lhs | rhs; }>(type);
    }

    void Builder::createXor(::Type* type) {
        binaryInsn<Opcodes::XOR, Opcodes::LXOR, [](i64 lhs, i64 rhs) { return lhs ^ rhs; }>(type);
    }

    void Builder::createShl(::Type* type) {
        binaryInsn<Opcodes::SHL, Opcodes::LSHL, [](i64 lhs, i64 rhs) { return (lhs << rhs); }>(type);
    }

    void Builder::createShr(::Type* type) {
        binaryInsn<Opcodes::SHR, Opcodes::LSHR, [](i64 lhs, i64 rhs) { return (lhs >> rhs); }>(type);
    }

    void Builder::createNot(::Type* type) {
        unaryInsn<Opcodes::NOT, Opcodes::LNOT, [](i64 operand) { return ~operand; }>(type);
    }

    void Builder::createNeg(::Type* type) {
        unaryInsn<Opcodes::NEG, Opcodes::LNEG, [](i64 operand) { return -operand; }>(type);
    }

    void Builder::createDup(::Type* type) {
        auto value = mContext.pop();

        assert(value.type == type->getRuntimeType());

        if (value.value) {
            mContext.push(value); // put it back and don't delete the origin, just ldc it again
            createLdc(type, value.value->value);
            return;
        }

        mContext.emplace(value.type);
        if (value.type == Type::Category1_Primitive) {
            mContext.push(value);
            mContext.push(value);
            insert<InsnNode>(Opcodes::DUP);
        } else {
            mContext.push(value);
            mContext.push(value);
            insert<InsnNode>(Opcodes::DUP2);
        }
    }

    void Builder::createSwap(::Type* topType, ::Type* bottomType) {
        auto top = mContext.pop();
        auto bottom = mContext.pop();

        assert(top.type == bottom.type && top.type == bottomType->getRuntimeType());

        if (top.value && bottom.value) {
            mInsertPoint->remove(top.value->origin);
            mInsertPoint->remove(bottom.value->origin);
            createLdc(topType, top.value->value);
            createLdc(bottomType, bottom.value->value);
            return; // does this work?
        }

        mContext.emplace(top.type);
        mContext.emplace(bottom.type);
        if (top.type == Type::Category1_Primitive) {
            insert<InsnNode>(Opcodes::DUP);
        } else {
            insert<InsnNode>(Opcodes::DUP2);
        }
    }

    void Builder::createLoad(::Type* type, u16 index) {
        Type rtType = type->getRuntimeType();

        mContext.emplace(rtType);

        switch (rtType) {
            case Type::Category1_Primitive:
                insert<VarInsnNode>(Opcodes::ILOAD, index);
                break;
            case Type::Category2_Primitive:
                insert<VarInsnNode>(Opcodes::LLOAD, index);
                break;
            case Type::Category2_Handle:
                insert<VarInsnNode>(Opcodes::HLOAD, index);
                break;
            case Type::Category2_Reference:
                if (index == 0) {
                    insert<InsnNode>(Opcodes::RLOAD_0);
                } else {
                    insert<VarInsnNode>(Opcodes::RLOAD, index);
                }

                break;
        }
    }

    void Builder::createStore(::Type* type, u16 index) {
        auto value = mContext.pop();

        assert(value.type == type->getRuntimeType());

        switch (value.type) {
            case Type::Category1_Primitive:
                insert<VarInsnNode>(Opcodes::ISTORE, index);
                break;
            case Type::Category2_Primitive:
                insert<VarInsnNode>(Opcodes::LSTORE, index);
                break;
            case Type::Category2_Handle:
                insert<VarInsnNode>(Opcodes::HSTORE, index);
                break;
            case Type::Category2_Reference:
                insert<VarInsnNode>(Opcodes::RSTORE, index);
                break;
        }
    }

    void Builder::createArrayLoad(::Type* type) {
        assert(false && "not implemented");
    }

    void Builder::createArrayStore(::Type* type) {
        assert(false && "not implemented");
    }

    void Builder::createArrayLength(::Type* type) {
        assert(false && "not implemented");
    }

    void Builder::createNew(::Type* type) {
        assert(type->isClassType());
        auto classType = static_cast<ClassType*>(type);

        insert<ClassInsnNode>(Opcodes::NEW, classType->getModuleName(), classType->getName());
    }

    void Builder::createNewArray(::Type* type) {
        assert(false && "not implemented");
    }

    void Builder::createIsInstance(::Type* checkedType) {
        auto value = mContext.pop();

        assert(value.type == Type::Category2_Reference && checkedType->isClassType());

        auto classType = static_cast<ClassType*>(checkedType);

        insert<ClassInsnNode>(Opcodes::ISINSTANCE, classType->getModuleName(), classType->getName());
    }

    void Builder::createGetField(::Type* ownerType, ::Type* type, std::string_view name) {
        auto object = mContext.pop();

        assert(object.type == Type::Category2_Reference && ownerType->isClassType());

        auto classType = static_cast<ClassType*>(ownerType);

        mContext.emplace(type->getRuntimeType());
        insert<FieldInsnNode>(Opcodes::GETFIELD, classType->getModuleName(), classType->getName(), name, type->getJesusASMType()->getDescriptor());
    }

    void Builder::createSetField(::Type* ownerType, ::Type* type, std::string_view name) {
        auto value = mContext.pop();
        auto object = mContext.pop();

        assert(value.type == type->getRuntimeType() && object.type == Type::Category2_Reference && ownerType->isClassType());

        auto classType = static_cast<ClassType*>(ownerType);

        insert<FieldInsnNode>(Opcodes::SETFIELD, classType->getModuleName(), classType->getName(), name, type->getJesusASMType()->getDescriptor());
    }

    void Builder::createCmp(::Type* type) {
        auto rhs = mContext.pop();
        auto lhs = mContext.pop();

        assert(lhs.type == rhs.type && lhs.type == type->getRuntimeType());

        if (lhs.value && rhs.value) {
            mInsertPoint->remove(lhs.value->origin);
            mInsertPoint->remove(rhs.value->origin);

            if (lhs.value->value < rhs.value->value) createLdc(::Type::Get("int"), -1);
            else if (lhs.value->value > rhs.value->value) createLdc(::Type::Get("int"), 1);
            else createLdc(::Type::Get("int"), (i64) 0);

            return;
        }

        mContext.emplace(Type::Category1_Primitive); // the result int

        switch (lhs.type) {
            case Type::Category1_Primitive:
                insert<InsnNode>(Opcodes::ICMP);
                break;
            case Type::Category2_Primitive:
                insert<InsnNode>(Opcodes::LCMP);
                break;
            case Type::Category2_Handle:
                insert<InsnNode>(Opcodes::HCMP);
                break;
            case Type::Category2_Reference:
                insert<InsnNode>(Opcodes::RCMP);
                break;
        }
    }

    void Builder::createJump(Label* label) {
        insert<JumpInsnNode>(Opcodes::JMP, label);
    }

    void Builder::createCondJumpEQ(Label* trueLabel, Label* falseLabel) {
        genericCondJump<Opcodes::JMPEQ, [](i64 cond) { return cond == 0; }>(trueLabel, falseLabel);
    }

    void Builder::createCondJumpNE(Label* trueLabel, Label* falseLabel) {
        genericCondJump<Opcodes::JMPNE, [](i64 cond) { return cond != 0; }>(trueLabel, falseLabel);
    }

    void Builder::createCondJumpLT(Label* trueLabel, Label* falseLabel) {
        genericCondJump<Opcodes::JMPLT, [](i64 cond) { return cond < 0; }>(trueLabel, falseLabel);
    }

    void Builder::createCondJumpGT(Label* trueLabel, Label* falseLabel) {
        genericCondJump<Opcodes::JMPGT, [](i64 cond) { return (cond > 0); }>(trueLabel, falseLabel);
    }

    void Builder::createCondJumpLE(Label* trueLabel, Label* falseLabel) {
        genericCondJump<Opcodes::JMPLE, [](i64 cond) { return cond <= 0; }>(trueLabel, falseLabel);
    }

    void Builder::createCondJumpGE(Label* trueLabel, Label* falseLabel) {
        genericCondJump<Opcodes::JMPGE, [](i64 cond) { return cond >= 0; }>(trueLabel, falseLabel);
    }

    void Builder::createLdc(::Type* type, i64 value) {
        //TODO: make this better and use the CONST_X instructions

        if (type->isIntegerType()) {
            auto size = static_cast<IntegerType*>(type)->getSize();
            switch (size) {
                case IntegerType::Size::Byte: {
                    auto narrowedValue = static_cast<i8>(value);
                    auto* origin = insert<IntInsnNode>(Opcodes::BPUSH, OperandSize::BYTE, narrowedValue);
                    mContext.emplace(Type::Category1_Primitive, ValueOrigin(origin, narrowedValue));
                    break;
                }

                case IntegerType::Size::Short: {
                    auto narrowedValue = static_cast<i16>(value);
                    auto* origin = insert<IntInsnNode>(Opcodes::SPUSH, OperandSize::SHORT, narrowedValue);
                    mContext.emplace(Type::Category1_Primitive, ValueOrigin(origin, narrowedValue));
                    break;
                }

                case IntegerType::Size::Int: {
                    auto narrowedValue = static_cast<i32>(value);
                    auto* origin = insert<IntInsnNode>(Opcodes::IPUSH, OperandSize::INT, narrowedValue);
                    mContext.emplace(Type::Category1_Primitive, ValueOrigin(origin, narrowedValue));
                    break;
                }

                case IntegerType::Size::Long: {
                    auto* origin = insert<IntInsnNode>(Opcodes::LPUSH, OperandSize::LONG, value);
                    mContext.emplace(Type::Category2_Primitive, ValueOrigin(origin, value));
                    break;
                }
            }

            return;
        }

        auto rtType = type->getRuntimeType();

        if (rtType == Type::Category1_Primitive) {
            if (value >= INT8_MIN && value <= INT8_MAX) {
                auto* origin = insert<IntInsnNode>(Opcodes::BPUSH, OperandSize::BYTE, value);
                mContext.emplace(Type::Category1_Primitive, ValueOrigin(origin, value));
            } else if (value >= INT16_MIN && value <= INT16_MAX) {
                auto* origin = insert<IntInsnNode>(Opcodes::SPUSH, OperandSize::SHORT, value);
                mContext.emplace(Type::Category1_Primitive, ValueOrigin(origin, value));
            } else { // no i64 check :tongue:
                value = static_cast<i32>(value);
                auto* origin = insert<IntInsnNode>(Opcodes::IPUSH, OperandSize::INT, value);
                mContext.emplace(Type::Category1_Primitive, ValueOrigin(origin, value));
            }
        } else if (rtType == Type::Category2_Primitive) {
            //TODO: maybe do casting at some point
            auto* origin = insert<IntInsnNode>(Opcodes::LPUSH, OperandSize::LONG, value);
            mContext.emplace(Type::Category2_Primitive, ValueOrigin(origin, value));
        } else {
            assert(false && "bad type");
        }
    }

    void Builder::createLdc(::Type* type, std::nullptr_t) {
        auto rtType = type->getRuntimeType();

        if (rtType == Type::Category2_Reference) {
            auto* origin = insert<InsnNode>(Opcodes::RCONST_NULL);
            mContext.emplace(Type::Category2_Reference, ValueOrigin(origin, 0));
        } else if (rtType == Type::Category2_Handle) {
            auto* origin = insert<InsnNode>(Opcodes::HCONST_NULL);
            mContext.emplace(Type::Category2_Handle, ValueOrigin(origin, 0));
        } else {
            assert(false && "bad type");
        }
    }

    void Builder::createLdc(std::string_view value) {
        mContext.emplace(Type::Category2_Reference);
        insert<LdcInsnNode>(value);
    }

    void Builder::createCast(::Type* from, ::Type* to) {
        if (from == to) return;

        auto value = mContext.pop();

        assert(value.type == from->getRuntimeType());

        if (value.value) {
            mInsertPoint->remove(value.value->origin);
            createLdc(to, value.value->value);
            return;
        }

        auto rtFrom = from->getRuntimeType();

        if (rtFrom == Type::Category1_Primitive) {
            if (to->isIntegerType()) {
                auto size = static_cast<IntegerType*>(to)->getSize();
                switch (size) {
                    case IntegerType::Size::Byte:
                        mContext.emplace(Type::Category1_Primitive);
                        insert<InsnNode>(Opcodes::I2B);
                        break;
                    case IntegerType::Size::Short:
                        mContext.emplace(Type::Category1_Primitive);
                        insert<InsnNode>(Opcodes::I2S);
                        break;
                    case IntegerType::Size::Int:
                        mContext.emplace(Type::Category1_Primitive);
                        break;
                    case IntegerType::Size::Long:
                        mContext.emplace(Type::Category2_Primitive);
                        insert<InsnNode>(Opcodes::I2L);
                        break;
                }
            } else {
                if (to->getRuntimeType() == Type::Category2_Primitive) {
                    mContext.emplace(Type::Category2_Primitive);
                    insert<InsnNode>(Opcodes::I2L);
                } else if (to->getRuntimeType() == Type::Category1_Primitive) {
                    mContext.emplace(Type::Category1_Primitive);
                } else {
                    assert(false && "bad type");
                }
            }
        } else if (rtFrom == Type::Category2_Primitive) {
            if (to->getRuntimeType() == Type::Category1_Primitive) {
                mContext.emplace(Type::Category2_Primitive);
                insert<InsnNode>(Opcodes::L2I);
            } else if (to->getRuntimeType() == Type::Category2_Primitive) {
                mContext.emplace(Type::Category2_Primitive);
            } else {
                assert(false && "bad type");
            }
        } else {
            assert(false && "bad type");
        }
    }

    void Builder::createCall(std::string_view moduleName, std::string_view name, FunctionType* type) { // TODO: eventually attempt to inline function calls. maybe in a separate optimization stage
        for (const auto& argument : type->getArgumentTypes()) {
            auto rtArgument = mContext.pop();
            assert(rtArgument.type == argument->getRuntimeType());
        }

        insert<CallInsnNode>(Opcodes::CALL, moduleName, name, type->getJesusASMType()->getDescriptor());
        if (!type->getReturnType()->isVoidType()) {
            mContext.emplace(type->getReturnType()->getRuntimeType());
        }
    }

    void Builder::createReturn(::Type* returnType) {
        if (returnType->isVoidType()) {
            insert<InsnNode>(Opcodes::RETURN);
            return;
        }

        auto rtType = returnType->getRuntimeType();
        auto value = mContext.pop();

        assert(value.type == rtType);

        switch (rtType) {
            case Type::Category1_Primitive:
                insert<InsnNode>(Opcodes::IRETURN);
                break;
            case Type::Category2_Primitive:
                insert<InsnNode>(Opcodes::LRETURN);
                break;
            case Type::Category2_Handle:
                insert<InsnNode>(Opcodes::HRETURN);
                break;
            case Type::Category2_Reference:
                insert<InsnNode>(Opcodes::RRETURN);
                break;
        }
    }

    void Builder::createBreakpoint() {
        insert<InsnNode>(Opcodes::BREAKPOINT);
    }

    void Builder::createReserve1() {
        insert<InsnNode>(Opcodes::RESERVE1);
    }

    void Builder::createReserve2() {
        insert<InsnNode>(Opcodes::RESERVE2);
    }
}