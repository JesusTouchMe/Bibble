// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_CODEGEN_PASS_COMMONPASS_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_CODEGEN_PASS_COMMONPASS_H 1

#include "Bibble/codegen/pass/Pass.h"

namespace codegen {
    class FinalizePass : public Pass {
    public:
        FinalizePass();

        void execute(ModuleNode* module) override;
    };

    class PrintPass : public Pass {
    public:
        PrintPass(std::ostream& out);

        void execute(ModuleNode* module) override;

    private:
        std::ostream& mOut;
    };

    class CodegenPass : public Pass {
    public:
        CodegenPass(std::string outputFileName);

        void execute(ModuleNode* module) override;

    private:
        std::string mOutputFileName;
    };
}

#endif // BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_CODEGEN_PASS_COMMONPASS_H
