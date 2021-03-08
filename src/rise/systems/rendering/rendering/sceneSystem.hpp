#pragma once

#include "util/ecs.hpp"
#include "state.hpp"
#include "systems/rendering/resources/model.hpp"
#include "systems/rendering/resources/mesh.hpp"

namespace rise::systems::rendering {
    // PARENT:Viewport
    void renderSceneSystem(flecs::iter it, ModelResource const* model, MeshResource const* mesh);
}
