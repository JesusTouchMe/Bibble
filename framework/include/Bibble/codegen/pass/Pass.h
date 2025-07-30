// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_CODEGEN_PASS_PASS_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_CODEGEN_PASS_PASS_H 1

#include "Bibble/codegen/Context.h"

namespace codegen {
    // Try to keep these in the order they should optimally be in a normal pass run
    enum class PassType {
        DeadCodeElimination,

        Finalize, // adds stuff like returns
        Print,
        Codegen,
    };

    class Pass {
    public:
        explicit Pass(PassType type);
        virtual ~Pass() = default;

        PassType getType() const;

        virtual void execute(ModuleNode* module) = 0;

    private:
        PassType mType;
    };

    class PassManager {
    public:
        void addPass(std::unique_ptr<Pass> pass);

        ssize_t findPass(PassType type) const;
        void insertPass(size_t position, std::unique_ptr<Pass> pass);
        void insertBefore(PassType other, std::unique_ptr<Pass> pass);
        void insertAfter(PassType other, std::unique_ptr<Pass> pass);
        void removePass(PassType type);

        void runPasses(ModuleNode* module);

    private:
        std::vector<std::unique_ptr<Pass>> mPasses;
    };
}

#endif // BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_CODEGEN_PASS_PASS_H
