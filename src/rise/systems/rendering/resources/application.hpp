#pragma once
#include "systems/rendering/rendering/state.hpp"
#include "systems/rendering/rendering/sceneState.hpp"
#include "systems/rendering/rendering/guiState.hpp"
#include "util/ecs.hpp"
#include <LLGL/LLGL.h>

namespace rise::systems::rendering {
    struct ApplicationResource {
        RenderState state;
        SceneState scene;
        GuiState gui;
    };
}
