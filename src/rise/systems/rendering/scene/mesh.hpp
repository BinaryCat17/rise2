#pragma once

#include "components/rendering/module.hpp"
#include "../core/resources.hpp"
#include "util/ecs.hpp"
#include "resources.hpp"

namespace rise::systems::rendering {
    using namespace components::rendering;

    void updateMesh(flecs::entity, CoreState &core, SceneState &scene, MeshRes &mesh,
            Path const &path);
}

