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

    std::unique_ptr<LLGL::RenderSystem> createRenderer();

    void initCoreState(flecs::entity, RenderSystem& renderer, Window& window, Context& context,
            Sampler& sampler, Queue& queue, CommandBuffer& cmdBuf, Path const &path,
            flecs::Name const &title, Extent2D size);
}
