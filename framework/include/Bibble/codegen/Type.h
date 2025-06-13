// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_CODEGEN_TYPE_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_CODEGEN_TYPE_H

namespace codegen {
    enum class Type {
        Primitive,
        Handle,
        Reference,

        CmpResult,
    };
}

#endif //BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_CODEGEN_TYPE_H
