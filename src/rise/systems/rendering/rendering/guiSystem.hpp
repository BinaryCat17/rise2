#pragma once

#include "util/ecs.hpp"
#include "state.hpp"
#include "systems/rendering/pipeline/state.hpp"

namespace rise::systems::rendering {

    GuiState createGuiRenderer(RenderState &render, std::string const &path);

    void initGuiRenderer(flecs::world &ecs, GuiState& state, RenderState &render);
}
