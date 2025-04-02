// Copyright 2025 JesusTouchMe

#include "Bibble/symbol/Scope.h"

#include "Bibble/type/BooleanType.h"
#include "Bibble/type/CharType.h"
#include "Bibble/type/ClassType.h"
#include "Bibble/type/HandleType.h"
#include "Bibble/type/IntegerType.h"
#include "Bibble/type/VoidType.h"

#include <unordered_map>

std::unordered_map<std::string, std::unique_ptr<Type>, StringViewHash, StringViewEqual> types;

void Type::Init() {
    JesusASM::Type::Init();

    types["byte"] = std::make_unique<IntegerType>("byte", IntegerType::Size::Byte);
    types["short"] = std::make_unique<IntegerType>("short", IntegerType::Size::Short);
    types["int"] = std::make_unique<IntegerType>("int", IntegerType::Size::Int);
    types["long"] = std::make_unique<IntegerType>("long", IntegerType::Size::Long);

    types["bool"] = std::make_unique<BooleanType>();
    types["char"] = std::make_unique<CharType>();
    types["handle"] = std::make_unique<HandleType>();
    types["void"] = std::make_unique<VoidType>();

    types["string"] = std::make_unique<ClassType>("std.Primitives", "String");
}

bool Type::Exists(std::string_view name) {
    return types.contains(name);
}

Type* Type::Get(std::string_view name) {
    auto type = types.find(name);
    if (type != types.end()) {
        return type->second.get();
    }
    return nullptr;
}