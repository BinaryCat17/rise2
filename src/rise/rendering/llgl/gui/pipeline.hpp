#pragma once

#include <glm/glm.hpp>
#include <LLGL/LLGL.h>

namespace rise::rendering::guiPipeline {
    struct Global {
        alignas(8) glm::vec2 scale;
        alignas(8) glm::vec2 translate;
    };

    LLGL::PipelineLayout *createLayout(LLGL::RenderSystem *renderer);

    LLGL::PipelineState *createPipeline(LLGL::RenderSystem *renderer,
            LLGL::PipelineLayout *layout, LLGL::ShaderProgram *program);
}