// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_HEADERBUILDER_H
#define BIBBLE_HEADERBUILDER_H 1

#include <string>
#include <vector>

class HeaderBuilder {
public:
    explicit HeaderBuilder(std::string moduleName);

    void addFunction(std::string name, std::string desc);
    void finalize();

    void emit(std::ofstream& out) const;

private:
    std::string mModuleName;

    std::string mBuffer;

    bool mFinalized = false;

    std::string escapeName(std::string_view name);
    std::string mangleName(std::string_view name, std::string_view desc);
    std::string descToCType(std::string_view desc);
    std::vector<std::string> parseFunctionType(std::string_view desc, std::string& returnTypeOut);
};

#endif // BIBBLE_HEADERBUILDER_H
