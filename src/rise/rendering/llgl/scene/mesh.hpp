#pragma once

#include <flecs.h>
#include "rendering/module.hpp"
#include "../scene/resources.hpp"
#include "resources.hpp"

namespace rise::rendering {
    void updateMesh(flecs::entity, CoreState &core, SceneState &scene, MeshRes &mesh, Path const &path);
}

