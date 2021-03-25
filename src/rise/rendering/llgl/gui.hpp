#pragma once
#include "resources.hpp"
#include "../imgui.hpp"

namespace rise::rendering {
    void importGuiState(flecs::world &ecs);

    void initGuiState(flecs::entity e, ApplicationState& state, Path const& path);

    void updateResources(flecs::entity, ApplicationId app, GuiContext context);

    void prepareImgui(flecs::entity, ApplicationId app, GuiContext context);

    void processImGui(flecs::entity, GuiContext context);

    void renderGui(flecs::entity, ApplicationId app, GuiContext context, Extent2D size);
}