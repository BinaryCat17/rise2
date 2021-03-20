#pragma once

#include <flecs.h>
#include <LLGL/LLGL.h>

namespace rise::rendering {
    struct Presets {
        flecs::entity material;
        flecs::entity mesh;
        flecs::entity texture;
    };

    struct SceneState {
        LLGL::PipelineLayout *layout = nullptr;
        LLGL::PipelineState *pipeline = nullptr;
        LLGL::VertexFormat format;
    };

    void importSceneState(flecs::world &ecs);
}