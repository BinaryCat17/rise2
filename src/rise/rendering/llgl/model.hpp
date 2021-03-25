#pragma once
#include "resources.hpp"

namespace rise::rendering {
    void importModel(flecs::world& ecs);

    void clearDescriptors(flecs::entity, ApplicationId app);

    void recreateDescriptors(flecs::entity, ApplicationId app);

    void updateTransform(flecs::entity, ApplicationId app);
}
