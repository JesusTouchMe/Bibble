// Copyright 2025 JesusTouchMe

#include "Bibble/codegen/pass/optimizer/DeadCodeElimination.h"

#include <list>
#include <JesusASM/tree/instructions/JumpInsnNode.h>

#include <queue>
#include <unordered_set>

namespace codegen {
    class DeadCodeEliminator {
    public:
        void eliminate(InsnList* list);
    };

    void DeadCodeEliminator::eliminate(InsnList* list) {
        if (list->getSize() == 0) return;

        AbstractInsnNode* first = list->getFirst();

        std::unordered_set<AbstractInsnNode*> reachable;
        std::unordered_set<LabelNode*> usedLabels;
        std::queue<AbstractInsnNode*> worklist;

        worklist.push(first);

        while (!worklist.empty()) {
            AbstractInsnNode* insn = worklist.front();
            worklist.pop();

            if (insn == nullptr || reachable.contains(insn)) continue;
            reachable.insert(insn);

            if (auto label = dynamic_cast<LabelNode*>(insn)) {
                usedLabels.insert(label);
            }

            if (auto jump = dynamic_cast<JumpInsnNode*>(insn)) {
                worklist.push(jump->getDestination());
                usedLabels.insert(jump->getDestination());

                if (!jump->isUnconditionalJump()) {
                    if (auto next = insn->getNext()) {
                        worklist.push(next);
                    }
                }

                continue;
            }

            if (insn->isUnconditionalJump()) continue; // atp it's only terminators here

            if (auto next = insn->getNext()) {
                worklist.push(next);
            }
        }

        for (AbstractInsnNode* insn = list->getFirst(); insn != nullptr;) {
            AbstractInsnNode* next = insn->getNext();

            if (!reachable.contains(insn)) {
                list->remove(insn);
            }

            insn = next;
        }

        for (AbstractInsnNode* insn = list->getFirst(); insn != nullptr;) {
            AbstractInsnNode* next = insn->getNext();

            if (auto label = dynamic_cast<LabelNode*>(insn)) {
                if (!usedLabels.contains(label)) {
                    list->remove(label);
                }
            }

            insn = next;
        }
    }

    DCEPass::DCEPass()
        : Pass(PassType::DeadCodeElimination) {}

    void DCEPass::execute(ModuleNode* module) {
        DeadCodeEliminator dce;

        for (const auto& func : module->functions) {
            dce.eliminate(&func->instructions);
        }
    }
}
