#pragma once

#include "resources.hpp"

namespace rise::rendering {
    void importCoreState(flecs::world &ecs);

    void initCoreRenderer(flecs::entity e, ApplicationState& state);

    void initCoreState(flecs::entity e, ApplicationState& state);

    void prepareRender(flecs::entity, ApplicationId app);

    void prepareColorPass(flecs::entity, ApplicationId app);

    void endColorPass(flecs::entity, ApplicationId app);

    void submitRender(flecs::entity, ApplicationId app);
}