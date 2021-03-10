#pragma once
#include <LLGL/LLGL.h>
#include "components/rendering/module.hpp"
#include "systems/rendering/resources/material.hpp"
#include "systems/rendering/resources/texture.hpp"
#include "systems/rendering/resources/viewport.hpp"

namespace rise::systems::rendering {
    struct ModelResource {
        LLGL::Buffer *objectBuffer = nullptr;
        LLGL::ResourceHeap *heap = nullptr;
        bool dirtyHeap = true;
        bool dirtyModel = true;
    };

    // PARENT: ApplicationResource
    void updateModelHeapSystem(flecs::iter it,  ModelResource* model, TextureResource* textures,
            MaterialResource* materials, ViewportResource* viewports);

    // PARENT: ApplicationResource
    void updateModelTransformSystem(flecs::iter it, ModelResource* model);
}
