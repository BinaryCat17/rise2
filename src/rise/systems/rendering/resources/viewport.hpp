#pragma once
#include <LLGL/LLGL.h>
#include "systems/rendering/pipeline/scene.hpp"

namespace rise::systems::rendering {
    struct ViewportResource {
        LLGL::Buffer *uniform = nullptr;
        scenePipeline::PerViewport *pData = nullptr;
        size_t currentLightId = 0;
        bool dirtyCamera = true;
        bool dirtyLight = true;
    };
}
