#pragma once
#include "state.hpp"
#include <glm/glm.hpp>

namespace rise::systems::rendering::guiPipeline {
    struct Global {
        alignas(8) glm::vec2 scale;
        alignas(8) glm::vec2 translate;
    };

    PipelineState create(LLGL::RenderSystem* renderer, std::string const& root);
}
