#pragma once
#include "resources.hpp"

namespace rise::rendering {
    void importSceneState(flecs::world &ecs);

    void initSceneState(flecs::entity e, ApplicationState& state, Path const& path);

    void renderScene(flecs::entity, ApplicationRef applicationRef, ViewportRef viewportRef,
            MeshId meshId, ModelId modelId);
}