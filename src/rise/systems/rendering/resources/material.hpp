#pragma once

#include <LLGL/LLGL.h>
#include "util/ecs.hpp"
#include "components/rendering/module.hpp"

namespace rise::systems::rendering {
    using namespace components::rendering;

    struct MaterialResource {
        LLGL::Buffer *uniform = nullptr;
    };

    // PARENT: ApplicationResource
    void updateMaterialSystem(flecs::iter it, MaterialResource* material,
                              DiffuseColor const* color);
}
