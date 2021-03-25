#pragma once
#include "resources.hpp"

namespace rise::rendering {
    void importMaterial(flecs::world &ecs);

    void updateMaterial(flecs::entity, ApplicationId app);
}