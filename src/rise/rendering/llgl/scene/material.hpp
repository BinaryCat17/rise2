#pragma once
#include <LLGL/LLGL.h>
#include <flecs.h>

namespace rise::rendering {
    struct MaterialRes {
        LLGL::Buffer *uniform = nullptr;
    };

    void importMaterial(flecs::world& ecs);
}
