#pragma once
#include <LLGL/LLGL.h>
#include "util/ecs.hpp"

namespace rise::systems::rendering {
    struct DiffuseTextureRes {
        flecs::entity e{};
    };

    struct MaterialRes {
        LLGL::Buffer* val = nullptr;
    };

    struct ViewportRes {
        LLGL::Buffer* val = nullptr;
    };

    struct TransformRes {
        LLGL::Buffer* val = nullptr;
    };

    struct SceneTag {};
}
