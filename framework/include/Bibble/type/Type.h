// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_TYPE_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_TYPE_H

#include "Bibble/codegen/Type.h"

#include "Bibble/diagnostic/Diagnostic.h"

#include <JesusASM/type/Type.h>

#include <moduleweb/types.h>

#include <memory>
#include <utility>

namespace symbol {
    class Scope;
}

class Type {
public:
    enum CodeType : u8 { // these are the type IDs used in bytecode for newarray (and maybe more) instructions
        T_BYTE,
        T_SHORT,
        T_INT,
        T_LONG,
        T_CHAR,
        T_FLOAT,
        T_DOUBLE,
        T_BOOL,
        T_HANDLE,
    };

    enum class CastLevel {
        Implicit,
        ImplicitWarning,
        Explicit,
        Disallowed
    };

    Type(std::string_view name) : mName(name) {}
    virtual ~Type() = default;

    virtual JesusASM::Type* getJesusASMType() const = 0;
    virtual codegen::Type getRuntimeType() const = 0;

    virtual CastLevel castTo(Type* destType) const = 0;
    virtual std::string getImplicitCastWarning(Type* destType) const { return ""; }
    virtual Type* replaceWith(Type* from, Type* to) { return this; }

    virtual void resolve(symbol::Scope* scope, diagnostic::Diagnostics& diag) {}

    virtual bool isIntegerType()    const { return false; }
    virtual bool isBooleanType()    const { return false; }
    virtual bool isCharType()       const { return false; }
    virtual bool isHandleType()     const { return false; }
    virtual bool isClassType()      const { return false; }
    virtual bool isClassView()      const { return false; }
    virtual bool isArrayType()      const { return false; }
    virtual bool isArrayView()      const { return false; }
    virtual bool isVoidType()       const { return false; }
    virtual bool isFunctionType()   const { return false; }
    virtual bool isViewType()       const { return false; }

    bool isReferenceType() const { return isClassType() || isArrayType(); }

    std::string_view getName() const { return mName; }

    static void Init();
    static bool Exists(std::string_view name);
    static Type* Get(std::string_view name);

protected:
    std::string mName;
};

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_TYPE_TYPE_H
