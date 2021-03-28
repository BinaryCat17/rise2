#pragma once

#include "resources.hpp"

namespace rise::rendering {
    void importShadowsState(flecs::world &ecs);

    void initShadowsState(flecs::entity e, ApplicationState &state);

    void prepareShadowPass(flecs::entity, ApplicationId app);

    void endShadowPass(flecs::entity, ApplicationId app);

    void updateLightMatrices(flecs::entity, ApplicationRef ref, LightId light, Position3D position);
}
