#pragma once

#include "../core/resources.hpp"
#include "components/rendering/imgui.hpp"
#include "components/rendering/module.hpp"
#include "util/ecs.hpp"
#include "resources.hpp"

namespace rise::systems::rendering {
    using namespace components::rendering;

    void initGuiState(flecs::entity e);

    void updateResources(flecs::entity, CoreState &core, GuiState &gui, GuiContext context);

    void prepareImgui(flecs::entity, CoreState &core, GuiState &gui, GuiContext context);

    void processImGui(flecs::entity, GuiContext context);

    void renderGui(flecs::entity, CoreState &core, GuiState &gui, GuiContext context,
            Extent2D size);
}
