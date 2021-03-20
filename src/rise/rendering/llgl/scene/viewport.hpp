#pragma once
#include <LLGL/LLGL.h>
#include <flecs.h>
#include "pipeline.hpp"

namespace rise::rendering {
    struct ViewportRes {
        LLGL::Buffer *uniform = nullptr;
        scenePipeline::PerViewport *pData = nullptr;
        unsigned lightId = 0;
    };

    void importMaterial(flecs::world& ecs);
}