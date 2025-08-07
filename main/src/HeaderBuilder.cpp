// Copyright 2025 JesusTouchMe

#include "HeaderBuilder.h"

#include "templates/HeaderEnd.h"
#include "templates/HeaderStart.h"

#include "Bibble/symbol/Scope.h"

#include <algorithm>
#include <format>
#include <fstream>
#include <unordered_map>

HeaderBuilder::HeaderBuilder(std::string moduleName)
    : mModuleName(std::move(moduleName)) {
    mBuffer.reserve(256);

    std::string headerGuard = escapeName(mModuleName);
    std::transform(headerGuard.begin(), headerGuard.end(), headerGuard.begin(), toupper);

    mBuffer += std::format(templates::HeaderStart, headerGuard);
}

void HeaderBuilder::addFunction(std::string name, std::string desc) {
    if (mFinalized) return;

    std::string returnType;
    std::vector<std::string> args = parseFunctionType(desc, returnType);
    std::string mangledName = mangleName(name, desc);

    mBuffer += std::format("JESUSVM_EXPORT {} {}(VMContext* ctx", descToCType(returnType), mangledName);

    for (auto& arg : args) {
        mBuffer += std::format(", {}", descToCType(arg));
    }

    mBuffer += ");\n\n";
}

void HeaderBuilder::finalize() {
    mBuffer += templates::HeaderEnd;
    mFinalized = true;
}

void HeaderBuilder::emit(std::ofstream& out) const {
    if (!mFinalized) return;
    out << mBuffer;
}

std::string HeaderBuilder::escapeName(std::string_view name) {
    std::string ret;
    ret.reserve(name.length());

    for (char c : name) {
        switch (c) {
            case '_': ret += "_1";  break;
            case '/': ret += "_";   break;
            case ';': ret += "_2";  break;
            case '[': ret += "_3";  break;
            case ':': ret += "_4";  break;
            default: ret += c;      break;
        }
    }

    return ret;
}

std::string HeaderBuilder::mangleName(std::string_view name, std::string_view desc) {
    std::string ret = std::format("JesusVM_{}_{}", escapeName(mModuleName), escapeName(name));

    std::string params;
    size_t start = desc.find('(');
    size_t end = desc.find(')');
    if (start == std::string::npos || end == std::string::npos || end <= start + 1)
        params = "";
    else
        params = desc.substr(start + 1, end - start - 1);

    if (!params.empty()) {
        ret += "__";
        ret += escapeName(params);
    }

    return ret;
}

std::string HeaderBuilder::descToCType(std::string_view desc) {
    static const std::unordered_map<std::string, std::string, StringViewHash, StringViewEqual> primitives = {
        {"V", "void"}, {"Z", "Bool"}, {"B", "Byte"}, {"S", "Short"},
        {"I", "Int"}, {"L", "Long"}, {"C", "Char"}, {"F", "Float"},
        {"D", "Double"}, {"H", "Handle"},
    };

    if (auto it = primitives.find(desc); it != primitives.end()) return it->second;
    if (desc[0] == '[') {
        std::string_view base = desc.substr(1);
        if (auto it = primitives.find(base); it != primitives.end()) return it->second + "Array";
        return "ObjectArray";
    }

    if (desc == "Rstd/Primitives:String;") return "String";
    return "JObject";
}

std::vector<std::string> HeaderBuilder::parseFunctionType(std::string_view desc, std::string& returnTypeOut) {
    std::vector<std::string> args;
    size_t i = 0;
    if (desc[i++] != '(') return args;

    while (desc[i] != ')') {
        std::string type;

        while (desc[i] == '[') {
            type += '[';
            i++;
        }

        if (desc[i] == 'R') {
            size_t end = desc.find(';', i);
            type += desc.substr(i, end - i + 1);
            i = end + 1;
        } else {
            type += desc[i++];
        }

        args.push_back(std::move(type));
    }
    i++;

    returnTypeOut = desc.substr(i);
    return args;
}
