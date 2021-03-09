#pragma once
#include "util/ecs.hpp"
#include "systems/rendering/resources/application.hpp"

namespace rise::systems::rendering {
    // PARENT: ApplicationResource
    void prepareRenderSystem(flecs::entity, ApplicationResource& app);

    // PARENT: ApplicationResource
    void submitRenderSystem(flecs::entity, ApplicationResource& app);
}
