// Copyright 2025 JesusTouchMe

#include "Bibble/symbol/Scope.h"

#include "Bibble/type/BooleanType.h"
#include "Bibble/type/CharType.h"
#include "Bibble/type/ClassType.h"
#include "Bibble/type/HandleType.h"
#include "Bibble/type/IntegerType.h"
#include "Bibble/type/VoidType.h"
#include "Bibble/type/Type.h"

#include <unordered_map>

bool init = false;

std::unordered_map<std::string, std::unique_ptr<Type>, StringViewHash, StringViewEqual> types;
std::unordered_map<std::string, Type*, StringViewHash, StringViewEqual> typeAliases;

void Type::Init() {
    if (init) return;

    JesusASM::Type::Init();

    types["byte"] = std::make_unique<IntegerType>("byte", IntegerType::Size::Byte);
    types["short"] = std::make_unique<IntegerType>("short", IntegerType::Size::Short);
    types["int"] = std::make_unique<IntegerType>("int", IntegerType::Size::Int);
    types["long"] = std::make_unique<IntegerType>("long", IntegerType::Size::Long);

    types["bool"] = std::make_unique<BooleanType>();
    types["boolean"] = std::make_unique<BooleanType>();
    types["char"] = std::make_unique<CharType>();
    types["handle"] = std::make_unique<HandleType>();
    types["void"] = std::make_unique<VoidType>();

    typeAliases["object"] = ClassType::Create("std/Primitives", "Object", nullptr);
    typeAliases["string"] = ClassType::Create("std/Primitives", "String", ClassType::Find("std/Primitives", "Object"));

    init = true;
}

bool Type::Exists(std::string_view name) {
    return types.contains(name) || typeAliases.contains(name);
}

Type* Type::Get(std::string_view name) {
    auto type = types.find(name);
    if (type != types.end()) {
        return type->second.get();
    }

    auto alias = typeAliases.find(name);
    if (alias != typeAliases.end()) {
        return alias->second;
    }

    return nullptr;
}