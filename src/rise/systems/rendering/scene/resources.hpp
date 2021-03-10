#pragma once
#include <LLGL/LLGL.h>

namespace rise::systems::rendering {

    struct DiffuseTextureRes {
        flecs::entity e{};
    };

    struct MaterialRes {
        LLGL::Buffer* val;
    };

    struct ViewportRes {
        LLGL::Buffer* val;
    };

    struct TransformRes {
        LLGL::Buffer* val;
    };
}
