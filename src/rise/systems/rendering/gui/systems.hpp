#pragma once

#include "../core/resources.hpp"
#include "components/rendering/imgui.hpp"
#include "components/rendering/module.hpp"
#include "util/ecs.hpp"
#include "resources.hpp"

namespace rise::systems::rendering {
    using namespace components::rendering;

    void initGui(flecs::entity e, RenderSystem const &renderer, Window window, Path const &path,
            Sampler sampler, PipelineLayout layout);

    void updateResources(flecs::entity, GuiContext context, RenderSystem const &renderer,
            GuiParameters shaderData, MeshRes &mesh, VertexFormat const &format);

    void prepareImgui(flecs::entity, GuiContext context, Window window);

    void processImGui(flecs::entity, GuiContext context);

    void renderGui(flecs::entity, GuiContext context, Pipeline pipeline, Extent2D size,
            RenderSystem const &renderer, CommandBuffer cmdBuf, MeshRes mesh, ResourceHeap heap);
}
