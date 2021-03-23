#pragma once
#include <LLGL/LLGL.h>
#include <flecs.h>

namespace rise::rendering {
    struct ModelRes {
        LLGL::Buffer *uniform = nullptr;
        LLGL::ResourceHeap *heap = nullptr;
    };

    void importModel(flecs::world& ecs);
}
