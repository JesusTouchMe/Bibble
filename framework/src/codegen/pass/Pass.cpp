// Copyright 2025 JesusTouchMe

#include "Bibble/codegen/pass/Pass.h"

#include <algorithm>

namespace codegen {
    Pass::Pass(PassType type)
        : mType(type) {}

    PassType Pass::getType() const {
        return mType;
    }

    void PassManager::addPass(std::unique_ptr<Pass> pass) {
        mPasses.push_back(std::move(pass));
    }

    ssize_t PassManager::findPass(PassType type) const {
        auto it = std::find_if(mPasses.begin(), mPasses.end(), [type](const auto& pass) {
            return pass->getType() == type;
        });
        if (it != mPasses.end()) return it - mPasses.begin();

        return -1;
    }

    void PassManager::insertPass(size_t position, std::unique_ptr<Pass> pass) {
        mPasses.insert(mPasses.begin() + position, std::move(pass));
    }
     void PassManager::insertBefore(PassType other, std::unique_ptr<Pass> pass) {
        auto position = findPass(other);
        if (position == -1) insertPass(0, std::move(pass));
        else insertPass(position, std::move(pass));
    }

    void PassManager::insertAfter(PassType other, std::unique_ptr<Pass> pass) {
        auto position = findPass(other);
        if (position == -1) addPass(std::move(pass));
        else insertPass(position + 1, std::move(pass));
    }

    void PassManager::removePass(PassType type) {
        std::erase_if(mPasses, [type](const auto& pass) {
            return pass->getType() == type;
        });
    }

    void PassManager::runPasses(ModuleNode* module) {
        for (auto& pass : mPasses) {
            pass->execute(module);
        }
    }
}
