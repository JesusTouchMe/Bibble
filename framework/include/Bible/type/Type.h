// Copyright 2025 JesusTouchMe

#ifndef BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_TYPE_H
#define BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_TYPE_H

#include <JesusASM/type/Type.h>

#include <memory>
#include <utility>

class Type {
public:
    enum class CastLevel {
        Implicit,
        ImplicitWarning,
        Explicit,
        Disallowed
    };

    Type(std::string_view name) : mName(name) {}
    virtual ~Type() = default;

    virtual int getStackSlots() const = 0;
    virtual JesusASM::Type* getJesusASMType() const = 0;

    virtual CastLevel castTo(Type* destType) const = 0;
    virtual std::string getImplicitCastWarning(Type* destType) const { return ""; }
    virtual Type* replaceWith(Type* from, Type* to) { return this; }

    virtual bool isIntegerType()    const { return false; }
    virtual bool isBooleanType()    const { return false; }
    virtual bool isCharType()       const { return false; }
    virtual bool isHandleType()     const { return false; }
    virtual bool isClassType()      const { return false; }
    virtual bool isVoidType()       const { return false; }
    virtual bool isFunctionType()   const { return false; }

    std::string_view getName() const { return mName; }

    static void Init();
    static bool Exists(std::string_view name);
    static Type* Get(std::string_view name);

protected:
    std::string mName;
};

#endif //BIBLE_FRAMEWORK_INCLUDE_BIBLE_TYPE_TYPE_H
