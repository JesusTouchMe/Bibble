// Copyright 2025 JesusTouchMe

#ifndef BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_CODEGEN_PASS_OPTIMIZER_DEADCODEELIMINATION_H
#define BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_CODEGEN_PASS_OPTIMIZER_DEADCODEELIMINATION_H 1

#include "Bibble/codegen/pass/Pass.h"

namespace codegen {
    class DCEPass : public Pass {
    public:
        DCEPass();

        void execute(ModuleNode* module) override;
    };
}

#endif // BIBBLE_FRAMEWORK_INCLUDE_BIBBLE_CODEGEN_PASS_OPTIMIZER_DEADCODEELIMINATION_H
