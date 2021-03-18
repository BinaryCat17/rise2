#pragma once

#include "../core/resources.hpp"
#include "rendering/imgui.hpp"
#include "rendering/module.hpp"
#include "resources.hpp"

namespace rise::rendering {
    void initGuiState(flecs::entity e);

    void updateResources(flecs::entity, CoreState &core, GuiState &gui, GuiContext context);

    void prepareImgui(flecs::entity, CoreState &core, GuiState &gui, GuiContext context);

    void processImGui(flecs::entity, GuiContext context);

    void renderGui(flecs::entity, CoreState &core, GuiState &gui, GuiContext context,
            Extent2D size);
}
