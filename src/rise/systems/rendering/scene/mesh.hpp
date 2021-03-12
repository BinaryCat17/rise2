#pragma once
#include "components/rendering/module.hpp"
#include "../core/resources.hpp"
#include "util/ecs.hpp"
#include "resources.hpp"

namespace rise::systems::rendering {
    using namespace components::rendering;

    void updateMesh(flecs::entity, RenderSystem & renderer, VertexFormat const& format,
            Mesh &mesh,  Path const &path);
}

