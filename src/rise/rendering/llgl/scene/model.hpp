#pragma once
#include "../core/texture.hpp"
#include <LLGL/LLGL.h>
#include <optional>

namespace rise::rendering {
    struct ModelRes {
        LLGL::Buffer *uniform = nullptr;
        LLGL::ResourceHeap *heap = nullptr;
    };

    void importModel(flecs::world& ecs);
}
