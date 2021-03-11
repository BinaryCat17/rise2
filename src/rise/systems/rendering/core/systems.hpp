#pragma once

#include "components/rendering/module.hpp"
#include "util/ecs.hpp"
#include "resources.hpp"
#include "texture.hpp"

namespace rise::systems::rendering {
    using namespace components::rendering;

    void prepareRender(flecs::entity, CommandBuffer cmdBuf, Context context);

    void submitRender(flecs::entity, CommandBuffer cmdBuf, Queue queue, Context context);

    void updateWindowSize(flecs::entity, Window window, Context context, Extent2D size);

    void initCoreState(flecs::entity e, Path const &path, flecs::Name const &title, Extent2D size);
}
