#pragma once

#include "resources.hpp"

namespace rise::rendering {
    void importShadowsState(flecs::world &ecs);

    void initShadowsState(flecs::entity, ApplicationState &state, Path const &path);

    void initShadowModels(flecs::entity, ApplicationId app);

    void removeShadowModels(flecs::entity, ApplicationId app);

    void updateShadowMaps(flecs::entity, ApplicationRef ref, ViewportRef viewportRef,
            LightId lightId);

    void updateLightUniforms(flecs::entity, ApplicationId app);

}
