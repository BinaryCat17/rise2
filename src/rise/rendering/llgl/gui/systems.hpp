#pragma once

#include "state.hpp"
#include "rendering/imgui.hpp"
#include "rendering/module.hpp"
#include "../core/state.hpp"

namespace rise::rendering {
    void updateResources(flecs::entity, CoreState &core, GuiState &gui, GuiContext context);

    void prepareImgui(flecs::entity, CoreState &core, GuiState &gui, GuiContext context);

    void processImGui(flecs::entity, GuiContext context);

    void renderGui(flecs::entity, CoreState &core, GuiState &gui, GuiContext context,
            Extent2D size);
}
