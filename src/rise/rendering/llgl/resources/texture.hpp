#pragma once

#include <flecs.h>
#include <LLGL/LLGL.h>

namespace rise::rendering {
    struct TextureRes {
        LLGL::Texture* val = nullptr;
    };

    void importTexture(flecs::world& ecs);
}
