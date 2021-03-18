#pragma once

#include "rise/rendering/module.hpp"
#include <memory>

namespace rise::rendering {
    struct LLGLModule {
        explicit LLGLModule(flecs::world &ecs);
    };

    struct LLGLApplication {};
}
