#pragma once

#include "resources.hpp"

namespace rise::rendering {
    void importPlatformState(flecs::world& ecs);

    void initPlatformWindow(flecs::entity e, ApplicationState& state, Title const& title);

    void initPlatformSurface(flecs::entity e, ApplicationState& state);

    void pullInputEvents(flecs::entity e, ApplicationId app, Extent2D& size);
}