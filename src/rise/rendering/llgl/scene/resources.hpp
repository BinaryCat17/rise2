#pragma once

#include <LLGL/LLGL.h>
#include "../core/resources.hpp"
#include "pipeline.hpp"
#include "util/ecs.hpp"

namespace rise::systems::rendering {


    struct MaterialRes {
        LLGL::Buffer *uniform = nullptr;
    };

    struct ViewportRes {
        LLGL::Buffer *uniform = nullptr;
        scenePipeline::PerViewport *pData = nullptr;
        unsigned lightId = 0;
        bool dirtyCamera = true;
        bool dirtyLight = true;
    };

    struct ModelRes {
        LLGL::Buffer *uniform = nullptr;
        LLGL::ResourceHeap *heap = nullptr;
    };

    struct SceneState {
        LLGL::PipelineLayout *layout = nullptr;
        LLGL::PipelineState *pipeline = nullptr;
        LLGL::VertexFormat format;
    };

    struct PointLightRes {};
}
