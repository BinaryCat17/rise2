#pragma once

#include "state.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "../core/state.hpp"
#include "../module.hpp"

namespace rise::rendering {
    void renderScene(flecs::entity, CoreState &core, SceneState &scene, Position2D position,
            Extent2D size, MeshRes mesh, ModelRes model);
}