#pragma once
#include "state.hpp"
#include "systems/rendering/pipeline/state.hpp"

namespace rise::systems::rendering {
    struct SceneState {
        PipelineState pipeline;
    };

    SceneState createSceneState(RenderState &render, std::string const &path);
}
