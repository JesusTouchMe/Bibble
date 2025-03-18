// Copyright 2025 JesusTouchMe

#include "Bible/symbol/Scope.h"

#include "Bible/type/BooleanType.h"
#include "Bible/type/CharType.h"
#include "Bible/type/ClassType.h"
#include "Bible/type/HandleType.h"
#include "Bible/type/IntegerType.h"
#include "Bible/type/VoidType.h"

#include <unordered_map>

std::unordered_map<std::string, std::unique_ptr<Type>, StringViewHash, StringViewEqual> types;

void Type::Init() {
    JesusASM::Type::Init();

    types["byte"] = std::make_unique<IntegerType>("byte", 1);
    types["short"] = std::make_unique<IntegerType>("short", 1);
    types["int"] = std::make_unique<IntegerType>("int", 1);
    types["long"] = std::make_unique<IntegerType>("long", 2);

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