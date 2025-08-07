// Copyright 2025 JesusTouchMe

#include "Bibble/codegen/Builder.h"

#include "Bibble/type/ArrayType.h"
#include "Bibble/type/ClassType.h"
#include "Bibble/type/IntegerType.h"
#include "Bibble/type/ViewType.h"

#include <ranges>

namespace codegen {
    Builder::Builder(Context& ctx)
        : mContext(ctx)
        , mInsertPoint(nullptr) {}

    void Builder::setInsertPoint(InsnList* insertPoint) {
        mInsertPoint = insertPoint;
        mContext.mVirtualStack.clear();
    }

    ClassNode* Builder::addClass(u16 modifiers, std::string_view name, std::string_view superModule, std::string_view superClass) const {
        mContext.getModule()->classes.push_back(std::make_unique<ClassNode>
                (modifiers, name, JesusASM::Name(superModule, superClass)));
        return mContext.getModule()->classes.back().get();
    }

    GlobalVarNode* Builder::addGlobalVar(u16 modifiers, std::string_view name, JesusASM::Type* type) const {
        mContext.getModule()->globals.push_back(std::make_unique<GlobalVarNode>(modifiers, name, type->getDescriptor()));
        return mContext.getModule()->globals.back().get();
    }

    FunctionNode* Builder::addFunction(u16 modifiers, std::string_view name, JesusASM::Type* type) const {
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
        binaryInsn<Opcodes::ADD, [](i64 lhs, i64 rhs) { return lhs + rhs; }>(type);
    }

    void Builder::createSub(::Type* type) {
        binaryInsn<Opcodes::SUB, [](i64 lhs, i64 rhs) { return lhs - rhs; }>(type);
    }

    void Builder::createMul(::Type* type) {
        binaryInsn<Opcodes::MUL, [](i64 lhs, i64 rhs) { return lhs * rhs; }>(type);
    }

    void Builder::createDiv(::Type* type) {
        binaryInsn<Opcodes::DIV, [](i64 lhs, i64 rhs) { return lhs / rhs; }>(type);
    }

    void Builder::createRem(::Type* type) {
        binaryInsn<Opcodes::REM, [](i64 lhs, i64 rhs) { return lhs % rhs; }>(type);
    }

    void Builder::createAnd(::Type* type) {
        binaryInsn<Opcodes::AND, [](i64 lhs, i64 rhs) { return lhs & rhs; }>(type);
    }

    void Builder::createOr(::Type* type) {
        binaryInsn<Opcodes::OR, [](i64 lhs, i64 rhs) { return lhs | rhs; }>(type);
    }

    void Builder::createXor(::Type* type) {
        binaryInsn<Opcodes::XOR, [](i64 lhs, i64 rhs) { return lhs ^ rhs; }>(type);
    }

    void Builder::createShl(::Type* type) {
        binaryInsn<Opcodes::SHL, [](i64 lhs, i64 rhs) { return (lhs << rhs); }>(type);
    }

    void Builder::createShr(::Type* type) {
        binaryInsn<Opcodes::SHR, [](i64 lhs, i64 rhs) { return (lhs >> rhs); }>(type);
    }

    void Builder::createNot(::Type* type) {
        unaryInsn<Opcodes::NOT, [](i64 operand) { return ~operand; }>(type);
    }

    void Builder::createNeg(::Type* type) {
        unaryInsn<Opcodes::NEG, [](i64 operand) { return -operand; }>(type);
    }

    void Builder::createPop(::Type* type) {
        auto value = mContext.pop();
        if (value.value) {
            mInsertPoint->remove(value.value->origin);
            return;
        }

        insert<InsnNode>(Opcodes::POP);
    }

    void Builder::createDup(::Type* type) {
        auto value = mContext.pop();

        assert(value.type == type->getRuntimeType());

        if (value.value) {
            mContext.push(value); // put it back and don't delete the origin, just ldc it again
            createLdc(type, value.value->value); // is this even efficient after the grand stack optimizing?
            return;
        }

        mContext.push(value);
        mContext.push(value);

        insert<InsnNode>(Opcodes::DUP);
    }

    void Builder::createDup2(::Type* type1, ::Type* type2) {
        auto value2 = mContext.pop();
        auto value1 = mContext.pop();

        assert(value1.type == type1->getRuntimeType());
        assert(value2.type == type2->getRuntimeType());

        if (value1.value && value2.value) {
            mContext.push(value1);
            mContext.push(value2);
            createLdc(type1, value1.value->value);
            createLdc(type2, value2.value->value);
            return;
        }

        mContext.push(value1);
        mContext.push(value2);
        mContext.push(value1);
        mContext.push(value2);

        insert<InsnNode>(Opcodes::DUP2);
    }

    void Builder::createDupX1(::Type* type) {
        auto top = mContext.pop();
        auto below = mContext.pop();

        assert(top.type == type->getRuntimeType());

        if (top.value && below.value) {
            mContext.push(top);
            mContext.push(below);
            createLdc(type, top.value->value); // again, is this gonna be more efficient than dup_x1? i need to run a lot of timing soon
            return;
        }


        mContext.push(top);
        mContext.push(below);
        mContext.push(top);

        insert<InsnNode>(Opcodes::DUP_X1);
    }

    void Builder::createDupX2(::Type* type) {
        auto top = mContext.pop();
        auto mid = mContext.pop();
        auto bottom = mContext.pop();

        assert(top.type == type->getRuntimeType());

        if (top.value && mid.value && bottom.value) {
            mContext.push(top);
            mContext.push(bottom);
            mContext.push(mid);
            createLdc(type, top.value->value); // again, is this gonna be more efficient than dup_x2? i need to run a lot of timing soon
            return;
        }


        mContext.push(top);
        mContext.push(bottom);
        mContext.push(mid);
        mContext.push(top);

        insert<InsnNode>(Opcodes::DUP_X2);
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

        insert<InsnNode>(Opcodes::SWAP);
    }

    void Builder::createInc(::Type* type, u16 index, i16 increment) {
        assert(type->getRuntimeType() == Type::Primitive);

        insert<IncInsnNode>(Opcodes::INC, index, increment);
    }

    void Builder::createLoad(::Type* type, u16 index) {
        Type rtType = type->getRuntimeType();

        mContext.emplace(rtType);

        switch (rtType) {
            case Type::Primitive:
                insert<VarInsnNode>(Opcodes::LOAD, index);
                break;
            case Type::Handle:
                insert<VarInsnNode>(Opcodes::HLOAD, index);
                break;
            case Type::Reference:
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
            case Type::Primitive:
                insert<VarInsnNode>(Opcodes::STORE, index);
                break;
            case Type::Handle:
                insert<VarInsnNode>(Opcodes::HSTORE, index);
                break;
            case Type::Reference:
                insert<VarInsnNode>(Opcodes::RSTORE, index);
                break;
        }
    }

    void Builder::createArrayLoad(::Type* _arrayType) {
        assert(_arrayType->isArrayView());

        ArrayType* arrayType;
        if (_arrayType->isViewType()) {
            arrayType = static_cast<ArrayType*>(static_cast<ViewType*>(_arrayType)->getBaseType());
        } else {
            arrayType = static_cast<ArrayType*>(_arrayType);
        }

        auto index = mContext.pop();
        auto arrayRef = mContext.pop();

        assert(index.type == Type::Primitive);
        assert(arrayRef.type == Type::Reference);

        if (arrayType->getElementType()->isIntegerType()) {
            auto integerType = static_cast<IntegerType*>(arrayType->getElementType());
            switch (integerType->getSize()) {
                case IntegerType::Size::Byte:
                    insert<InsnNode>(Opcodes::BALOAD);
                    mContext.emplace(Type::Primitive);
                    break;
                case IntegerType::Size::Short:
                    insert<InsnNode>(Opcodes::SALOAD);
                    mContext.emplace(Type::Primitive);
                    break;
                case IntegerType::Size::Int:
                    insert<InsnNode>(Opcodes::IALOAD);
                    mContext.emplace(Type::Primitive);
                    break;
                case IntegerType::Size::Long:
                    insert<InsnNode>(Opcodes::LALOAD);
                    mContext.emplace(Type::Primitive);
                    break;
            }
        } else if (arrayType->getElementType()->isCharType()) {
            insert<InsnNode>(Opcodes::CALOAD);
            mContext.emplace(Type::Primitive);
        } else if (arrayType->getElementType()->isBooleanType()) {
            insert<InsnNode>(Opcodes::BALOAD);
            mContext.emplace(Type::Primitive);
        } else if (arrayType->getElementType()->isHandleType()) {
            insert<InsnNode>(Opcodes::HALOAD);
            mContext.emplace(Type::Handle);
        } else if (arrayType->getElementType()->isClassType() || arrayType->getElementType()->isArrayType()) {
            insert<InsnNode>(Opcodes::RALOAD);
            mContext.emplace(Type::Reference);
        } else {
            assert(false && "Unsupported type");
        }
    }

    void Builder::createArrayStore(::Type* _arrayType) {
        assert(_arrayType->isArrayType());

        auto* arrayType = static_cast<ArrayType*>(_arrayType);

        auto value = mContext.pop();
        auto index = mContext.pop();
        auto arrayRef = mContext.pop();

        assert(index.type == Type::Primitive);
        assert(arrayRef.type == Type::Reference);

        if (arrayType->getElementType()->isIntegerType()) {
            auto integerType = static_cast<IntegerType*>(arrayType->getElementType());
            switch (integerType->getSize()) {
                case IntegerType::Size::Byte:
                    insert<InsnNode>(Opcodes::BASTORE);
                    break;
                case IntegerType::Size::Short:
                    insert<InsnNode>(Opcodes::SASTORE);
                    break;
                case IntegerType::Size::Int:
                    insert<InsnNode>(Opcodes::IASTORE);
                    break;
                case IntegerType::Size::Long:
                    insert<InsnNode>(Opcodes::LASTORE);
                    break;
            }
        } else if (arrayType->getElementType()->isCharType()) {
            insert<InsnNode>(Opcodes::CASTORE);
        } else if (arrayType->getElementType()->isBooleanType()) {
            insert<InsnNode>(Opcodes::BASTORE);
        } else if (arrayType->getElementType()->isHandleType()) {
            insert<InsnNode>(Opcodes::HASTORE);
        } else if (arrayType->getElementType()->isClassType() || arrayType->getElementType()->isArrayType()) {
            insert<InsnNode>(Opcodes::RASTORE);
        } else {
            assert(false && "Unsupported type");
        }
    }

    void Builder::createArrayLength(::Type* arrayType) {
        assert(arrayType->isArrayView());

        auto arrayRef = mContext.pop();
        assert(arrayRef.type == Type::Reference);

        insert<InsnNode>(Opcodes::ARRAYLENGTH);
        mContext.emplace(Type::Primitive);
    }

    void Builder::createNew(::Type* type) {
        assert(type->isClassType());
        auto classType = static_cast<ClassType*>(type);

        insert<ClassInsnNode>(Opcodes::NEW, classType->getModuleName(), classType->getName());
        mContext.emplace(Type::Reference);
    }

    void Builder::createNewArray(::Type* _arrayType) {
        assert(_arrayType->isArrayType());

        auto* arrayType = static_cast<ArrayType*>(_arrayType);

        auto length = mContext.pop();

        assert(length.type == Type::Primitive);

        if (arrayType->getElementType()->isClassType()) {
            auto* classType = static_cast<ClassType*>(arrayType->getElementType());
            insert<ClassInsnNode>(Opcodes::RNEWARRAY, classType->getModuleName(), classType->getName());
            mContext.emplace(Type::Reference);

            return;
        } else if (arrayType->getElementType()->isArrayType()) {
            //TODO: support runtime multidimensional arrays (T[][]) and compile-time multidimensional arrays (T[,])
            assert(false && "unimplemented");
        }

        if (arrayType->getElementType()->isIntegerType()) {
            auto integerType = static_cast<IntegerType*>(arrayType->getElementType());
            switch (integerType->getSize()) {
                case IntegerType::Size::Byte:
                    insert<IntInsnNode>(Opcodes::NEWARRAY, OperandSize::BYTE, ::Type::T_BYTE);
                    mContext.emplace(Type::Reference);
                    break;
                case IntegerType::Size::Short:
                    insert<IntInsnNode>(Opcodes::NEWARRAY, OperandSize::BYTE, ::Type::T_SHORT);
                    mContext.emplace(Type::Reference);
                    break;
                case IntegerType::Size::Int:
                    insert<IntInsnNode>(Opcodes::NEWARRAY, OperandSize::BYTE, ::Type::T_INT);
                    mContext.emplace(Type::Reference);
                    break;
                case IntegerType::Size::Long:
                    insert<IntInsnNode>(Opcodes::NEWARRAY, OperandSize::BYTE, ::Type::T_LONG);
                    mContext.emplace(Type::Reference);
                    break;
            }
        } else if (arrayType->getElementType()->isCharType()) {
            insert<IntInsnNode>(Opcodes::NEWARRAY, OperandSize::BYTE, ::Type::T_CHAR);
            mContext.emplace(Type::Reference);
        } else if (arrayType->getElementType()->isBooleanType()) {
            insert<IntInsnNode>(Opcodes::NEWARRAY, OperandSize::BYTE, ::Type::T_BOOL);
            mContext.emplace(Type::Reference);
        } else if (arrayType->getElementType()->isHandleType()) {
            insert<IntInsnNode>(Opcodes::NEWARRAY, OperandSize::BYTE, ::Type::T_HANDLE);
            mContext.emplace(Type::Reference);
        } else {
            assert(false && "Unsupported type");
        }
    }

    void Builder::createIsInstance(::Type* checkedType) {
        auto value = mContext.pop();

        assert(value.type == Type::Reference && checkedType->isClassType());

        auto classType = static_cast<ClassType*>(checkedType);

        insert<ClassInsnNode>(Opcodes::ISINSTANCE, classType->getModuleName(), classType->getName());
    }

    void Builder::createGetField(::Type* ownerType, ::Type* type, std::string_view name) {
        auto object = mContext.pop();

        assert(object.type == Type::Reference && ownerType->isClassType());

        auto classType = static_cast<ClassType*>(ownerType);

        mContext.emplace(type->getRuntimeType());
        insert<FieldInsnNode>(Opcodes::GETFIELD, classType->getModuleName(), classType->getName(), name, type->getJesusASMType()->getDescriptor());
    }

    void Builder::createSetField(::Type* ownerType, ::Type* type, std::string_view name) {
        auto value = mContext.pop();
        auto object = mContext.pop();

        assert(value.type == type->getRuntimeType() && object.type == Type::Reference && ownerType->isClassType());

        auto classType = static_cast<ClassType*>(ownerType);

        insert<FieldInsnNode>(Opcodes::SETFIELD, classType->getModuleName(), classType->getName(), name, type->getJesusASMType()->getDescriptor());
    }

    void Builder::createGetGlobal(::Type* type, std::string_view moduleName, std::string_view name) {
        mContext.emplace(type->getRuntimeType());
        insert<GlobalVarInsnNode>(Opcodes::GETGLOBAL, moduleName, name, type->getJesusASMType()->getDescriptor());
    }

    void Builder::createSetGlobal(::Type* type, std::string_view moduleName, std::string_view name) {
        auto value = mContext.pop();

        assert(value.type == type->getRuntimeType());

        insert<GlobalVarInsnNode>(Opcodes::SETGLOBAL, moduleName, name, type->getJesusASMType()->getDescriptor());
    }

    void Builder::createCmpEQ(::Type* type) {
        cmpInsn<CmpOperator::EQ>(type);
    }

    void Builder::createCmpNE(::Type* type) {
        cmpInsn<CmpOperator::NE>(type);
    }

    void Builder::createCmpLT(::Type* type) {
        cmpInsn<CmpOperator::LT>(type);
    }

    void Builder::createCmpGT(::Type* type) {
        cmpInsn<CmpOperator::GT>(type);
    }

    void Builder::createCmpLE(::Type* type) {
        cmpInsn<CmpOperator::LE>(type);
    }

    void Builder::createCmpGE(::Type* type) {
        cmpInsn<CmpOperator::GE>(type);
    }

    void Builder::createJump(Label* label) {
        insert<JumpInsnNode>(Opcodes::JMP, label);
    }

    void Builder::createCondJump(Label* trueLabel, Label* falseLabel) {
        auto cmp = mContext.pop();

        if (cmp.type == Type::CmpResult) {
            assert(cmp.cmpOperator.has_value());

            switch (*cmp.cmpOperator) {
                case CmpOperator::EQ:
                    insert<JumpInsnNode>(Opcodes::JMPEQ, trueLabel);
                    insert<JumpInsnNode>(Opcodes::JMP, falseLabel);
                    break;
                case CmpOperator::NE:
                    insert<JumpInsnNode>(Opcodes::JMPNE, trueLabel);
                    insert<JumpInsnNode>(Opcodes::JMP, falseLabel);
                    break;
                case CmpOperator::LT:
                    insert<JumpInsnNode>(Opcodes::JMPLT, trueLabel);
                    insert<JumpInsnNode>(Opcodes::JMP, falseLabel);
                    break;
                case CmpOperator::GT:
                    insert<JumpInsnNode>(Opcodes::JMPGT, trueLabel);
                    insert<JumpInsnNode>(Opcodes::JMP, falseLabel);
                    break;
                case CmpOperator::LE:
                    insert<JumpInsnNode>(Opcodes::JMPLE, trueLabel);
                    insert<JumpInsnNode>(Opcodes::JMP, falseLabel);
                    break;
                case CmpOperator::GE:
                    insert<JumpInsnNode>(Opcodes::JMPGE, trueLabel);
                    insert<JumpInsnNode>(Opcodes::JMP, falseLabel);
                    break;
            }
        }
    }

    void Builder::createJumpCmpEQ(::Type* type, Label* label) {
        auto rhs = mContext.pop();
        auto lhs = mContext.pop();

        assert(lhs.type == rhs.type && type->getRuntimeType() == lhs.type);

        if (lhs.value && rhs.value) {
            if (lhs.value->value == rhs.value->value) {
                mInsertPoint->remove(lhs.value->origin);
                mInsertPoint->remove(rhs.value->origin);
                createJump(label);
            }
        }

        if (lhs.type == Type::Reference) {
            insert<JumpInsnNode>(Opcodes::JMP_RCMPEQ, label);
        } else if (lhs.type == Type::Handle) {
            insert<JumpInsnNode>(Opcodes::JMP_HCMPEQ, label);
        } else if (lhs.type == Type::Primitive) {
            insert<JumpInsnNode>(Opcodes::JMP_CMPEQ, label);
        } else {
            assert(false && "bad type");
        }
    }

    void Builder::createJumpCmpNE(::Type* type, Label* label) {
        auto rhs = mContext.pop();
        auto lhs = mContext.pop();

        assert(lhs.type == rhs.type && type->getRuntimeType() == lhs.type);

        if (lhs.value && rhs.value) {
            if (lhs.value->value != rhs.value->value) {
                mInsertPoint->remove(lhs.value->origin);
                mInsertPoint->remove(rhs.value->origin);
                createJump(label);
            }
        }

        if (lhs.type == Type::Reference) {
            insert<JumpInsnNode>(Opcodes::JMP_RCMPNE, label);
        } else if (lhs.type == Type::Handle) {
            insert<JumpInsnNode>(Opcodes::JMP_HCMPNE, label);
        } else if (lhs.type == Type::Primitive) {
            insert<JumpInsnNode>(Opcodes::JMP_CMPNE, label);
        } else {
            assert(false && "bad type");
        }
    }

    void Builder::createJumpCmpLT(::Type* type, Label* label) {
        auto rhs = mContext.pop();
        auto lhs = mContext.pop();

        assert(lhs.type == rhs.type && type->getRuntimeType() == lhs.type);

        if (lhs.value && rhs.value) {
            if (lhs.value->value < rhs.value->value) {
                mInsertPoint->remove(lhs.value->origin);
                mInsertPoint->remove(rhs.value->origin);
                createJump(label);
            }
        }

        if (lhs.type == Type::Primitive) {
            insert<JumpInsnNode>(Opcodes::JMP_CMPLT, label);
        } else {
            assert(false && "bad type");
        }
    }

    void Builder::createJumpCmpGT(::Type* type, Label* label) {
        auto rhs = mContext.pop();
        auto lhs = mContext.pop();

        assert(lhs.type == rhs.type && type->getRuntimeType() == lhs.type);

        if (lhs.value && rhs.value) {
            if (lhs.value->value > rhs.value->value) {
                mInsertPoint->remove(lhs.value->origin);
                mInsertPoint->remove(rhs.value->origin);
                createJump(label);
            }
        }

        if (lhs.type == Type::Primitive) {
            insert<JumpInsnNode>(Opcodes::JMP_CMPGT, label);
        } else {
            assert(false && "bad type");
        }
    }

    void Builder::createJumpCmpLE(::Type* type, Label* label) {
        auto rhs = mContext.pop();
        auto lhs = mContext.pop();

        assert(lhs.type == rhs.type && type->getRuntimeType() == lhs.type);

        if (lhs.value && rhs.value) {
            if (lhs.value->value <= rhs.value->value) {
                mInsertPoint->remove(lhs.value->origin);
                mInsertPoint->remove(rhs.value->origin);
                createJump(label);
            }
        }

        if (lhs.type == Type::Primitive) {
            insert<JumpInsnNode>(Opcodes::JMP_CMPLE, label);
        } else {
            assert(false && "bad type");
        }
    }

    void Builder::createJumpCmpGE(::Type* type, Label* label) {
        auto rhs = mContext.pop();
        auto lhs = mContext.pop();

        assert(lhs.type == rhs.type && type->getRuntimeType() == lhs.type);

        if (lhs.value && rhs.value) {
            if (lhs.value->value >= rhs.value->value) {
                mInsertPoint->remove(lhs.value->origin);
                mInsertPoint->remove(rhs.value->origin);
                createJump(label);
            }
        }

        if (lhs.type == Type::Primitive) {
            insert<JumpInsnNode>(Opcodes::JMP_CMPGE, label);
        } else {
            assert(false && "bad type");
        }
    }

    void Builder::createLdc(::Type* type, i64 value) {
        // we will not give a shit about the type if we can use a super optimized const instruction
        switch (value) {
            case -1: {
                auto* origin = insert<InsnNode>(Opcodes::CONST_M1);
                mContext.emplace(Type::Primitive, ValueOrigin(origin, value));
                return;
            }
            case 0: {
                auto* origin = insert<InsnNode>(Opcodes::CONST_0);
                mContext.emplace(Type::Primitive, ValueOrigin(origin, value));
                return;
            }
            case 1: {
                auto* origin = insert<InsnNode>(Opcodes::CONST_1);
                mContext.emplace(Type::Primitive, ValueOrigin(origin, value));
                return;
            }
            case 2: {
                auto* origin = insert<InsnNode>(Opcodes::CONST_2);
                mContext.emplace(Type::Primitive, ValueOrigin(origin, value));
                return;
            }
            case 3: {
                auto* origin = insert<InsnNode>(Opcodes::CONST_3);
                mContext.emplace(Type::Primitive, ValueOrigin(origin, value));
                return;
            }
            case 4: {
                auto* origin = insert<InsnNode>(Opcodes::CONST_4);
                mContext.emplace(Type::Primitive, ValueOrigin(origin, value));
                return;
            }
        }

        if (type->isIntegerType()) {
            auto size = static_cast<IntegerType*>(type)->getSize();
            switch (size) {
                case IntegerType::Size::Byte: {
                    auto narrowedValue = static_cast<i8>(value);
                    auto* origin = insert<IntInsnNode>(Opcodes::BPUSH, OperandSize::BYTE, narrowedValue);
                    mContext.emplace(Type::Primitive, ValueOrigin(origin, narrowedValue));
                    break;
                }

                case IntegerType::Size::Short: {
                    auto narrowedValue = static_cast<i16>(value);
                    auto* origin = insert<IntInsnNode>(Opcodes::SPUSH, OperandSize::SHORT, narrowedValue);
                    mContext.emplace(Type::Primitive, ValueOrigin(origin, narrowedValue));
                    break;
                }

                case IntegerType::Size::Int: {
                    auto narrowedValue = static_cast<i32>(value);
                    auto* origin = insert<IntInsnNode>(Opcodes::IPUSH, OperandSize::INT, narrowedValue);
                    mContext.emplace(Type::Primitive, ValueOrigin(origin, narrowedValue));
                    break;
                }

                case IntegerType::Size::Long: {
                    auto* origin = insert<IntInsnNode>(Opcodes::LPUSH, OperandSize::LONG, value);
                    mContext.emplace(Type::Primitive, ValueOrigin(origin, value));
                    break;
                }
            }

            return;
        }

        auto rtType = type->getRuntimeType();

        if (rtType == Type::Primitive) {
            if (value >= INT8_MIN && value <= INT8_MAX) {
                auto* origin = insert<IntInsnNode>(Opcodes::BPUSH, OperandSize::BYTE, value);
                mContext.emplace(Type::Primitive, ValueOrigin(origin, value));
            } else if (value >= INT16_MIN && value <= INT16_MAX) {
                auto* origin = insert<IntInsnNode>(Opcodes::SPUSH, OperandSize::SHORT, value);
                mContext.emplace(Type::Primitive, ValueOrigin(origin, value));
            } else { // no i64 check :tongue:
                value = static_cast<i32>(value);
                auto* origin = insert<IntInsnNode>(Opcodes::IPUSH, OperandSize::INT, value);
                mContext.emplace(Type::Primitive, ValueOrigin(origin, value));
            }
        } else {
            assert(false && "bad type");
        }
    }

    void Builder::createLdc(::Type* type, bool value) {
        assert(type->getRuntimeType() == Type::Primitive);

        if (value) {
            auto* origin = insert<InsnNode>(Opcodes::CONST_1);
            mContext.emplace(Type::Primitive, ValueOrigin(origin, 1));
        } else {
            auto* origin = insert<InsnNode>(Opcodes::CONST_0);
            mContext.emplace(Type::Primitive, ValueOrigin(origin, 0));
        }
    }

    void Builder::createLdc(::Type* type, std::nullptr_t) {
        auto rtType = type->getRuntimeType();

        if (rtType == Type::Reference) {
            auto* origin = insert<InsnNode>(Opcodes::RCONST_NULL);
            mContext.emplace(Type::Reference, ValueOrigin(origin, 0));
        } else if (rtType == Type::Handle) {
            auto* origin = insert<InsnNode>(Opcodes::HCONST_NULL);
            mContext.emplace(Type::Handle, ValueOrigin(origin, 0));
        } else {
            assert(false && "bad type");
        }
    }

    void Builder::createLdc(std::string_view value) {
        mContext.emplace(Type::Reference);
        insert<LdcInsnNode>(value);
    }

    void Builder::createCast(::Type* from, ::Type* to) { // TODO: make this better please
        if (from == to) return;

        if (to->isVoidType()) {
            createPop(from);
            return;
        }

        auto value = mContext.pop();

        assert(value.type == from->getRuntimeType());

        if (value.value) {
            mInsertPoint->remove(value.value->origin);

            if (to->isVoidType()) {
                createPop(from);
            } else {
                if (to->isReferenceType() && value.value->value == 0) createLdc(to, nullptr);
                else createLdc(to, value.value->value);
            }
            return;
        }

        auto rtFrom = from->getRuntimeType();

        if (rtFrom == Type::Primitive) {
            if (to->isIntegerType()) {
                auto size = static_cast<IntegerType*>(to)->getSize();
                switch (size) {
                    case IntegerType::Size::Byte:
                        mContext.emplace(Type::Primitive);
                        insert<InsnNode>(Opcodes::L2B);
                        break;
                    case IntegerType::Size::Short:
                        mContext.emplace(Type::Primitive);
                        insert<InsnNode>(Opcodes::L2S);
                        break;
                    case IntegerType::Size::Int:
                        mContext.emplace(Type::Primitive);
                        insert<InsnNode>(Opcodes::L2I);
                        break;
                    case IntegerType::Size::Long:
                        mContext.emplace(Type::Primitive);
                        break;
                }
            } else {
                if (to->getRuntimeType() == Type::Primitive) {
                    mContext.emplace(Type::Primitive);
                } else {
                    assert(false && "bad type");
                }
            }
        } else if (rtFrom == Type::Reference && to->getRuntimeType() == Type::Reference) {
            mContext.emplace(Type::Reference); // TODO: allow casting up (object to string for example) with runtime check
        } else {
            assert(false && "bad type");
        }
    }

    void Builder::createCall(std::string_view moduleName, std::string_view name, FunctionType* type) { // TODO: eventually attempt to inline function calls. maybe in a separate optimization stage
        for (auto* argument : std::ranges::reverse_view(type->getArgumentTypes())) {
            auto rtArgument = mContext.pop();
            assert(rtArgument.type == argument->getRuntimeType());
        }

        insert<CallInsnNode>(Opcodes::CALL, moduleName, name, type->getJesusASMType()->getDescriptor());
        if (!type->getReturnType()->isVoidType()) {
            mContext.emplace(type->getReturnType()->getRuntimeType());
        }
    }

    void Builder::createVirtualCall(ClassType* ownerClass, std::string_view name, FunctionType* type) {
        for (auto* argument : std::ranges::reverse_view(type->getArgumentTypes())) {
            auto rtArgument = mContext.pop();
            assert(rtArgument.type == argument->getRuntimeType());
        }

        std::vector<::Type*> argTypes = type->getArgumentTypes();
        argTypes.erase(argTypes.begin());
        FunctionType* methodType = FunctionType::Create(type->getReturnType(), std::move(argTypes));

        insert<MethodInsnNode>(Opcodes::CALLVIRTUAL, std::string(ownerClass->getModuleName()), std::string(ownerClass->getName()),
            std::string(name), std::string(methodType->getJesusASMType()->getDescriptor()));
        if (!methodType->getReturnType()->isVoidType()) {
            mContext.emplace(methodType->getReturnType()->getRuntimeType());
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
            case Type::Primitive:
                insert<InsnNode>(Opcodes::LRETURN);
                break;
            case Type::Handle:
                insert<InsnNode>(Opcodes::HRETURN);
                break;
            case Type::Reference:
                insert<InsnNode>(Opcodes::RRETURN);
                break;
        }
    }
}
