#pragma once

#include "components/rendering/module.hpp"
#include "util/ecs.hpp"
#include "resources.hpp"

namespace rise::systems::rendering {
    using namespace components::rendering;

    void updateTexture(flecs::entity e, RenderSystem &renderer, Texture &texture,
            Path const &path);
}
