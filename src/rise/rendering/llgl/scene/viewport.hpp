#pragma once
#include <LLGL/LLGL.h>
#include "pipeline.hpp"

namespace rise::rendering {
    struct ViewportRes {
        LLGL::Buffer *uniform = nullptr;
        scenePipeline::PerViewport *pData = nullptr;
        unsigned lightId = 0;
    };

    void updateViewportCamera(ViewportRes &viewport, glm::vec2 size, glm::vec3 position, glm::vec3 rotation);

    void writeViewportLight(ViewportRes &viewport, glm::vec3 position, glm::vec3 color,
            float intensity, float distance);

    void finishViewport(LLGL::RenderSystem *renderer, ViewportRes &viewport);
}