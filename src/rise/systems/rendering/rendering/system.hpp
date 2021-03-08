#pragma once
#include "util/ecs.hpp"

namespace rise::systems::rendering {
    // PARENT: ApplicationResource
    void prepareRenderSystem(flecs::iter it);

    // PARENT: ApplicationResource
    void submitRenderSystem(flecs::iter it);
}
