#pragma once

#include "rise/rendering/module.hpp"
#include <memory>

namespace rise::rendering {
    struct LLGLModule {
        explicit LLGLModule(flecs::world &ecs);

        static void reg(flecs::entity app);
    };
}
