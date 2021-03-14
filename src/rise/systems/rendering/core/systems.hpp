#pragma once

#include "components/rendering/module.hpp"
#include "util/ecs.hpp"
#include "resources.hpp"
#include "texture.hpp"

namespace rise::systems::rendering {
    using namespace components::rendering;

    void prepareRender(flecs::entity, CoreState& core);

    void submitRender(flecs::entity, CoreState& core);

    void updateWindowSize(flecs::entity, CoreState& core, Extent2D size);

    void initCoreState(flecs::entity e);

    void updateRelative(flecs::entity, CoreState& core, Relative val);
}
