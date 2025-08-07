// Copyright 2025 JesusTouchMe

#include "Bibble/codegen/pass/CommonPass.h"

#include <JesusASM/tree/instructions/InsnNode.h>

namespace codegen {
    FinalizePass::FinalizePass()
        : Pass(PassType::Finalize) {}

    void FinalizePass::execute(ModuleNode* module) {
        for (auto& func : module->functions) {
            if (!(func->modifiers & MODULEWEB_FUNCTION_MODIFIER_NATIVE) && !func->instructions.hasTerminator()) {
                func->instructions.add(std::make_unique<InsnNode>(JesusASM::Opcodes::RETURN));
            }
        }
    }

    PrintPass::PrintPass(std::ostream& out)
        : Pass(PassType::Print)
        , mOut(out) {}

    void PrintPass::execute(ModuleNode* module) {
        module->print(mOut);
    }

    CodegenPass::CodegenPass(std::string outputFileName)
        : Pass(PassType::Codegen)
        , mOutputFileName(std::move(outputFileName)) {}

    void CodegenPass::execute(ModuleNode* module) {
        moduleweb::ModuleBuilder moduleBuilder;
        moduleweb::ModuleInfo moduleInfo;

        module->emit(moduleBuilder, moduleInfo);

        moduleweb::FileOutStream out(mOutputFileName);

        moduleInfo.emit(out);
    }
}
