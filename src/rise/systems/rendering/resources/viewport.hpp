#pragma once
#include <LLGL/LLGL.h>
#include "components/rendering/module.hpp"
#include "systems/rendering/pipeline/scene.hpp"

namespace rise::systems::rendering {
    using namespace components::rendering;

    struct ViewportResource {
        LLGL::Buffer *uniform = nullptr;
        scenePipeline::PerViewport *pData = nullptr;
        size_t currentLightId = 0;
        bool dirtyCamera = true;
        bool dirtyLight = true;
    };

    // PARENT: ApplicationResource
    void prepareViewportSystem(flecs::iter it, ViewportResource* viewport);

    void updateCameraSystem(flecs::entity, ViewportResource& viewport, Position3D pos,
                            Rotation3D rotation, Extent2D viewportSize);

    void updateLightSystem(flecs::entity, ViewportResource& viewport, Position3D pos,
                           Distance distance, DiffuseColor color, Intensity intensity);

    // PARENT: ApplicationResource
    void finishViewportSystem(flecs::iter, ViewportResource* viewport);
}
