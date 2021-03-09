#pragma once

#include "util/ecs.hpp"
#include "systems/rendering/resources/application.hpp"

namespace rise::systems::rendering {
    // PARENT:Application
    void renderSceneSystem(flecs::entity e, ApplicationResource& app);
}
