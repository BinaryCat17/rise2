#pragma once

#include <glm/glm.hpp>
#include "resources.hpp"
#include "components/rendering/module.hpp"
#include "util/ecs.hpp"
#include "../core/resources.hpp"

namespace rise::systems::rendering::guiPipeline {
    struct Global {
        alignas(8) glm::vec2 scale;
        alignas(8) glm::vec2 translate;
    };
}

namespace rise::systems::rendering {
    using namespace components::rendering;

    void initGuiPipeline(flecs::entity e);
}
