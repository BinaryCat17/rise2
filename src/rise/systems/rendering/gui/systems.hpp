#pragma once

#include "../core/resources.hpp"
#include "components/rendering/imgui.hpp"
#include "components/rendering/module.hpp"
#include "util/ecs.hpp"
#include "resources.hpp"

namespace rise::systems::rendering {
    using namespace components::rendering;

    void initGui(flecs::entity e);

    void updateResources(flecs::entity, GuiContext context, RenderSystem &renderer,
            GuiParameters shaderData, Mesh &mesh, VertexFormat const &format);

    void prepareImgui(flecs::entity, GuiContext context, Window window);

    void processImGui(flecs::entity, GuiContext context);

    void renderGui(flecs::entity, GuiContext context, Pipeline pipeline, Extent2D size,
            RenderSystem &renderer, CommandBuffer cmdBuf, Mesh mesh, ResourceHeap heap);
}
